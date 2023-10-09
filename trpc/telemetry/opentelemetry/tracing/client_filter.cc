//
//
// Tencent is pleased to support the open source community by making tRPC available.
//
// Copyright (C) 2023 THL A29 Limited, a Tencent company.
// All rights reserved.
//
// If you have downloaded a copy of the tRPC source code from Tencent,
// please note that tRPC source code is licensed under the  Apache 2.0 License,
// A copy of the Apache 2.0 License is included in this file.
//
//

#include "trpc/telemetry/opentelemetry/tracing/client_filter.h"

#include <unordered_map>

#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/tracer.h"
#include "trpc/codec/http/http_protocol.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"
#include "trpc/telemetry/opentelemetry/tracing/text_map_carrier.h"

namespace trpc {

namespace opentelemetry {

/// @brief A map used to store ClientTextMapCarrierFunc for different protocol.
///        key: protocol name, value: ClientTextMapCarrierFunc
std::unordered_map<std::string, ClientTextMapCarrierFunc> client_carrier_funcs_map;

void SetClientCarrierFunc(const std::string& protocol_name, const ClientTextMapCarrierFunc& carrier_func) {
  if (carrier_func == nullptr) {
    TRPC_LOG_ERROR("cat not set client carrier func of " << protocol_name << " with nullptr");
    return;
  }
  client_carrier_funcs_map[protocol_name] = carrier_func;
}

ClientTextMapCarrierFunc GetClientCarrierFunc(const std::string& protocol_name) {
  if (client_carrier_funcs_map.find(protocol_name) != client_carrier_funcs_map.end()) {
    return client_carrier_funcs_map[protocol_name];
  }
  // If there is no corresponding implementation set for the protocol, the implementation constructed through transinfo
  // is used by default.
  return ClientTransInfoCarrierFunc;
}

TextMapCarrierPtr ClientTransInfoCarrierFunc(const ClientContextPtr& context) {
  return std::make_unique<TransInfoWriter>(context->GetMutablePbReqTransInfo());
}

TextMapCarrierPtr ClientHttpCarrierFunc(const ClientContextPtr& context) {
  auto http_req = static_cast<HttpRequestProtocol*>(context->GetRequest().get())->request;
  return std::make_unique<HttpHeaderWriter>(http_req.get());
}

}  // namespace opentelemetry

int OpenTelemetryTracingClientFilter::Init() {
  auto telemetry = TelemetryFactory::GetInstance()->Get(trpc::opentelemetry::kOpenTelemetryTelemetryName);
  if (!telemetry) {
    TRPC_FMT_ERROR("plugin {} is not registered", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }

  tracer_factory_ = trpc::dynamic_pointer_cast<OpenTelemetryTracing>(telemetry->GetTracing());
  auto& config = tracer_factory_->GetConfig();
  disable_trace_body_ = config.traces_config.disable_trace_body;
  deferred_sample_error_ = config.traces_config.enable_deferred_sample & config.traces_config.deferred_sample_error;
  use_grpc_reported_ = (config.protocol == "grpc");

  // initializes the ClientCarrierFunc for each protocol.
  trpc::opentelemetry::SetClientCarrierFunc("trpc", trpc::opentelemetry::ClientTransInfoCarrierFunc);
  trpc::opentelemetry::SetClientCarrierFunc("http", trpc::opentelemetry::ClientHttpCarrierFunc);

  return 0;
}

std::vector<FilterPoint> OpenTelemetryTracingClientFilter::GetFilterPoint() {
  std::vector<FilterPoint> points = {FilterPoint::CLIENT_PRE_RPC_INVOKE, FilterPoint::CLIENT_POST_RPC_INVOKE};
  return points;
}

void OpenTelemetryTracingClientFilter::operator()(FilterStatus& status, FilterPoint point,
                                                  const ClientContextPtr& context) {
  status = FilterStatus::CONTINUE;
  if (!tracer_factory_) {
    return;
  }

  // avoid self-looping reporting of the grpc trace exporter
  if (use_grpc_reported_ && context->GetCalleeName() == trpc::opentelemetry::kGrpcTraceExporterServiceName) {
    return;
  }

  // ensures that client_span is not empty
  ClientTracingSpan* client_span = context->GetFilterData<ClientTracingSpan>(tracer_factory_->GetPluginID());
  if (!client_span) {
    context->SetFilterData(tracer_factory_->GetPluginID(), ClientTracingSpan());
    client_span = context->GetFilterData<ClientTracingSpan>(tracer_factory_->GetPluginID());
  }

  if (point == FilterPoint::CLIENT_PRE_RPC_INVOKE) {
    client_span->span = std::move(NewSpan(context, client_span->parent_span));
  } else if (point == FilterPoint::CLIENT_POST_RPC_INVOKE) {
    FinishSpan(client_span->span, context);
  }
}

::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> OpenTelemetryTracingClientFilter::GetTracer(
    const ClientContextPtr& context) {
  std::string msg("");
  auto tracer = tracer_factory_->MakeTracer(context->GetCallerName().c_str(), msg);
  return tracer;
}

trpc::opentelemetry::OpenTelemetryTracingSpanPtr OpenTelemetryTracingClientFilter::NewSpan(
    const ClientContextPtr& context, const std::any& parent_span) {
  // constructs start options
  ::opentelemetry::trace::StartSpanOptions op;
  if (parent_span.type() == typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr)) {
    const auto& parent_span_ptr = std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(parent_span);
    if (parent_span_ptr.get()) {
      op.parent = parent_span_ptr->GetContext();
    }
  }
  op.kind = ::opentelemetry::trace::SpanKind::kClient;
  std::unordered_map<std::string, std::string> attributes;
  if (trpc::opentelemetry::GetClientTraceAttrsFunc()) {
    trpc::opentelemetry::GetClientTraceAttrsFunc()(context, context->GetRequestData(), attributes);
  }

  // creates span
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> tracer = GetTracer(context);
  auto span = tracer->StartSpan(context->GetFuncName(), attributes, op);

  // sets attributes
  span->SetAttribute(trpc::opentelemetry::kTraceCalleeService, context->GetCalleeName());
  span->SetAttribute(trpc::opentelemetry::kTraceCalleeMethod, context->GetFuncName());
  span->SetAttribute(trpc::opentelemetry::kTraceCallerService, context->GetCallerName());
  span->SetAttribute(trpc::opentelemetry::kTraceCallerMethod, context->GetCallerFuncName());
  span->SetAttribute(trpc::opentelemetry::kTraceHostIp, TrpcConfig::GetInstance()->GetGlobalConfig().local_ip);
  span->SetAttribute(trpc::opentelemetry::kTraceNamespace, TrpcConfig::GetInstance()->GetGlobalConfig().env_namespace);
  span->SetAttribute(trpc::opentelemetry::kTraceEnvName, TrpcConfig::GetInstance()->GetGlobalConfig().env_name);
  if (context->IsDyeingMessage()) {
    span->SetAttribute(trpc::opentelemetry::kTraceDyeingKey, context->GetDyeingKey());
  }

  // injects trace information into request
  ::opentelemetry::trace::propagation::HttpTraceContext http_ctx;
  const ::opentelemetry::nostd::string_view span_key(::opentelemetry::trace::kSpanKey);
  ::opentelemetry::context::Context otlp_ctx(span_key, span);
  trpc::opentelemetry::ClientTextMapCarrierFunc carrier_func =
      trpc::opentelemetry::GetClientCarrierFunc(context->GetCodecName());
  trpc::opentelemetry::TextMapCarrierPtr carrier = carrier_func(context);
  http_ctx.Inject(*carrier, otlp_ctx);

  return span;
}

void OpenTelemetryTracingClientFilter::FinishSpan(const std::any& any_span, const ClientContextPtr& context) {
  if (any_span.type() != typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr) ||
      std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(any_span).get() == nullptr) {
    return;
  }

  const auto& span = std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(any_span);

  if (!span->IsRecording()) {
    return;
  }

  span->SetAttribute(trpc::opentelemetry::kTracePeerIp, context->GetIp());
  span->SetAttribute(trpc::opentelemetry::kTracePeerPort, context->GetPort());

  // reports request and response
  if (NeedReportReqRsp(context, span)) {
    AddRequestEvent(context, span);
    AddRsponseEvent(context, span);
  }

  // sets status
  trpc::opentelemetry::detail::SetStatus(context, span);

  span->End();
}

void OpenTelemetryTracingClientFilter::AddRequestEvent(const ClientContextPtr& context,
                                                       const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span) {
  // only processing protobuf data currently
  if (context->GetRequestData() && context->GetReqEncodeType() == TrpcContentEncodeType::TRPC_PROTO_ENCODE) {
    auto* request = static_cast<const google::protobuf::Message*>(context->GetRequestData());
    std::string json_request;
    trpc::opentelemetry::detail::GetMsgJsonData(request, json_request);
    span->AddEvent("SENT",
                   {{"message.uncompressed_size", json_request.length()}, {"message.detail", std::move(json_request)}});
  }
}

void OpenTelemetryTracingClientFilter::AddRsponseEvent(const ClientContextPtr& context,
                                                       const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span) {
  // only processing protobuf data currently
  if (context->GetResponseData() && context->GetRspEncodeType() == TrpcContentEncodeType::TRPC_PROTO_ENCODE) {
    auto* response = static_cast<const google::protobuf::Message*>(context->GetResponseData());
    std::string json_response;
    trpc::opentelemetry::detail::GetMsgJsonData(response, json_response);
    span->AddEvent("RECEIVED", {{"message.uncompressed_size", json_response.length()},
                                {"message.detail", std::move(json_response)}});
  }
}

bool OpenTelemetryTracingClientFilter::NeedReportReqRsp(const ClientContextPtr& context,
                                                        const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span) {
  // the reporting switch must be turned on in order to report data
  if (disable_trace_body_) {
    return false;
  }
  // if it hits sampling, then report
  if (span->GetContext().IsSampled()) {
    return true;
  }
  // if it enables deferred sample error and happens to meet error, then report
  if (deferred_sample_error_ && !context->GetStatus().OK()) {
    return true;
  }
  return false;
}

}  // namespace trpc
