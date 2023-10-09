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

#include "trpc/telemetry/opentelemetry/tracing/server_filter.h"

#include <unordered_map>

#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "opentelemetry/trace/tracer.h"
#include "trpc/codec/http/http_protocol.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/server/service.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/tracing/tracing_filter_index.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"
#include "trpc/telemetry/opentelemetry/tracing/text_map_carrier.h"

namespace trpc {

namespace opentelemetry {

/// @brief A map used to store ServerTextMapCarrierFunc for different protocol.
///        key: protocol name, value: ServerTextMapCarrierFunc
std::unordered_map<std::string, ServerTextMapCarrierFunc> server_carrier_funcs_map;

void SetServerCarrierFunc(const std::string& protocol_name, const ServerTextMapCarrierFunc& carrier_func) {
  if (carrier_func == nullptr) {
    TRPC_LOG_ERROR("cat not set server carrier func of " << protocol_name << " with nullptr");
    return;
  }
  server_carrier_funcs_map[protocol_name] = carrier_func;
}

ServerTextMapCarrierFunc GetServerCarrierFunc(const std::string& protocol_name) {
  if (server_carrier_funcs_map.find(protocol_name) != server_carrier_funcs_map.end()) {
    return server_carrier_funcs_map[protocol_name];
  }
  // If there is no corresponding implementation set for the protocol, the implementation constructed through transinfo
  // is used by default.
  return ServerTransInfoCarrierFunc;
}

TextMapCarrierPtr ServerTransInfoCarrierFunc(const ServerContextPtr& context) {
  return std::make_unique<TransInfoReader>(context->GetPbReqTransInfo());
}

TextMapCarrierPtr ServerHttpCarrierFunc(const ServerContextPtr& context) {
  auto http_req = static_cast<HttpRequestProtocol*>(context->GetRequestMsg().get())->request;
  return std::make_unique<HttpHeaderReader>(*http_req);
}

}  // namespace opentelemetry

int OpenTelemetryTracingServerFilter::Init() {
  auto telemetry = TelemetryFactory::GetInstance()->Get(trpc::opentelemetry::kOpenTelemetryTelemetryName);
  if (!telemetry) {
    TRPC_FMT_ERROR("plugin {} is not registered", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }
  tracer_factory_ = trpc::dynamic_pointer_cast<OpenTelemetryTracing>(telemetry->GetTracing());
  auto& config = tracer_factory_->GetConfig();
  disable_trace_body_ = config.traces_config.disable_trace_body;
  deferred_sample_error_ = config.traces_config.enable_deferred_sample & config.traces_config.deferred_sample_error;

  // initializes the ServerCarrierFunc for each protocol.
  trpc::opentelemetry::SetServerCarrierFunc("trpc", trpc::opentelemetry::ServerTransInfoCarrierFunc);
  trpc::opentelemetry::SetServerCarrierFunc("http", trpc::opentelemetry::ServerHttpCarrierFunc);

  return 0;
}

std::vector<FilterPoint> OpenTelemetryTracingServerFilter::GetFilterPoint() {
  std::vector<FilterPoint> points = {FilterPoint::SERVER_PRE_RPC_INVOKE, FilterPoint::SERVER_POST_RPC_INVOKE};
  return points;
}

void OpenTelemetryTracingServerFilter::operator()(FilterStatus& status, FilterPoint point,
                                                  const ServerContextPtr& context) {
  status = FilterStatus::CONTINUE;
  if (!tracer_factory_) {
    return;
  }

  if (point == FilterPoint::SERVER_PRE_RPC_INVOKE) {
    ServerTracingSpan svr_span;
    svr_span.span = std::move(NewSpan(context));
    context->SetFilterData<ServerTracingSpan>(tracer_factory_->GetPluginID(), std::move(svr_span));
  } else if (point == FilterPoint::SERVER_POST_RPC_INVOKE) {
    ServerTracingSpan* ptr = context->GetFilterData<ServerTracingSpan>(tracer_factory_->GetPluginID());
    if (ptr) {
      FinishSpan(ptr->span, context);
    }
  }
}

::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> OpenTelemetryTracingServerFilter::GetTracer(
    const ServerContextPtr& context) {
  std::string msg;
  auto tracer = tracer_factory_->MakeTracer(context->GetCalleeName().c_str(), msg);
  return tracer;
}

trpc::opentelemetry::OpenTelemetryTracingSpanPtr OpenTelemetryTracingServerFilter::NewSpan(
    const ServerContextPtr& context) {
  // extracts upstream trace information
  ::opentelemetry::context::Context otel_context;
  ::opentelemetry::trace::propagation::HttpTraceContext http_ctx;
  trpc::opentelemetry::ServerTextMapCarrierFunc carrier_get_func =
      trpc::opentelemetry::GetServerCarrierFunc(context->GetCodecName());
  trpc::opentelemetry::TextMapCarrierPtr carrier = carrier_get_func(context);
  auto cli_ctx = http_ctx.Extract(*carrier, otel_context);
  auto ctx_val = cli_ctx.GetValue(::opentelemetry::trace::kSpanKey);
  auto parent_span =
      ::opentelemetry::nostd::get<::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>>(ctx_val);

  // constructs start options
  ::opentelemetry::trace::StartSpanOptions op;
  op.parent = parent_span->GetContext();
  op.kind = ::opentelemetry::trace::SpanKind::kServer;
  std::unordered_map<std::string, std::string> attributes;
  if (trpc::opentelemetry::GetServerTraceAttrsFunc()) {
    trpc::opentelemetry::GetServerTraceAttrsFunc()(context, context->GetRequestData(), attributes);
  }

  // creates span
  auto tracer = GetTracer(context);
  auto span = tracer->StartSpan(context->GetFuncName(), attributes, op);

  // sets attributes
  span->SetAttribute(trpc::opentelemetry::kTraceCalleeService, context->GetCalleeName());
  span->SetAttribute(trpc::opentelemetry::kTraceCalleeMethod, context->GetFuncName());
  span->SetAttribute(trpc::opentelemetry::kTraceCallerService, context->GetCallerName());
  span->SetAttribute(trpc::opentelemetry::kTraceCallerMethod, "");
  span->SetAttribute(trpc::opentelemetry::kTraceHostIp, context->GetService()->GetServiceAdapterOption().ip);
  span->SetAttribute(trpc::opentelemetry::kTraceHostPort, context->GetService()->GetServiceAdapterOption().port);
  span->SetAttribute(trpc::opentelemetry::kTracePeerIp, context->GetIp());
  span->SetAttribute(trpc::opentelemetry::kTracePeerPort, context->GetPort());
  span->SetAttribute(trpc::opentelemetry::kTraceNamespace, TrpcConfig::GetInstance()->GetGlobalConfig().env_namespace);
  span->SetAttribute(trpc::opentelemetry::kTraceEnvName, TrpcConfig::GetInstance()->GetGlobalConfig().env_name);
  if (context->IsDyeingMessage()) {
    span->SetAttribute(trpc::opentelemetry::kTraceDyeingKey, context->GetDyeingKey());
  }

  return span;
}

void OpenTelemetryTracingServerFilter::FinishSpan(const std::any& any_span, const ServerContextPtr& context) {
  if (any_span.type() != typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr) ||
      std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(any_span).get() == nullptr) {
    return;
  }
  const auto& span = std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(any_span);

  if (!span->IsRecording()) {
    return;
  }

  // reports request and response
  if (NeedReportReqRsp(context, span)) {
    AddRequestEvent(context, span);
    AddRsponseEvent(context, span);
  }

  // sets status
  trpc::opentelemetry::detail::SetStatus(context, span);

  span->End();
}

void OpenTelemetryTracingServerFilter::AddRequestEvent(const ServerContextPtr& context,
                                                       const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span) {
  // only processing protobuf data currently
  if (context->GetRequestData() && context->GetReqEncodeType() == TrpcContentEncodeType::TRPC_PROTO_ENCODE) {
    auto* request = static_cast<const google::protobuf::Message*>(context->GetRequestData());
    std::string json_request;
    trpc::opentelemetry::detail::GetMsgJsonData(request, json_request);
    span->AddEvent("RECEIVED",
                   {{"message.uncompressed_size", json_request.length()}, {"message.detail", std::move(json_request)}});
  }
}

void OpenTelemetryTracingServerFilter::AddRsponseEvent(const ServerContextPtr& context,
                                                       const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span) {
  // only processing protobuf data currently
  if (context->GetResponseData() && context->GetRspEncodeType() == TrpcContentEncodeType::TRPC_PROTO_ENCODE) {
    auto* response = static_cast<const google::protobuf::Message*>(context->GetResponseData());
    std::string json_response;
    trpc::opentelemetry::detail::GetMsgJsonData(response, json_response);
    span->AddEvent(
        "SENT", {{"message.uncompressed_size", json_response.length()}, {"message.detail", std::move(json_response)}});
  }
}

bool OpenTelemetryTracingServerFilter::NeedReportReqRsp(const ServerContextPtr& context,
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
