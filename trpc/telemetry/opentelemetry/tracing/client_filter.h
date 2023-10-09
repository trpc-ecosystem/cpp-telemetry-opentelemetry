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

#pragma once

#include <string>
#include <vector>

#include "trpc/client/client_context.h"
#include "trpc/filter/filter.h"
#include "trpc/tracing/tracing_filter_index.h"

#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"
#include "trpc/telemetry/opentelemetry/tracing/text_map_carrier.h"

namespace trpc {

namespace opentelemetry {

/// @brief The client-side TextMapCarrier retrieval function which returns a specific implementation of TextMapCarrier
///        based on the ClientContextPtr.
/// @param context ClientContext
/// @return a specific implementation of TextMapCarrier
using ClientTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ClientContextPtr& context)>;

/// @brief Sets a client-side TextMapCarrier retrieval function for a specific protocol.
/// @param protocol_name protocol name
/// @param carrier_func TextMapCarrier retrieval function
void SetClientCarrierFunc(const std::string& protocol_name, const ClientTextMapCarrierFunc& carrier_func);

/// @brief Gets a client-side TextMapCarrier retrieval function for a specific protocol.
/// @param protocol_name protocol name
/// @return TextMapCarrier retrieval function
ClientTextMapCarrierFunc GetClientCarrierFunc(const std::string& protocol_name);

/// @brief The implementation function for constructing a TextMapCarrier using transinfo.
TextMapCarrierPtr ClientTransInfoCarrierFunc(const ClientContextPtr& context);

/// @brief The implementation function for constructing a TextMapCarrier using HttpRequest.
TextMapCarrierPtr ClientHttpCarrierFunc(const ClientContextPtr& context);

}  // namespace opentelemetry

class OpenTelemetryTracingClientFilter : public trpc::MessageClientFilter {
 public:
  int Init() override;

  std::string Name() override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  std::vector<FilterPoint> GetFilterPoint() override;

  void operator()(FilterStatus& status, FilterPoint point, const ClientContextPtr& context) override;

 private:
  // Gets a tracer from tracer_factory_
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> GetTracer(const ClientContextPtr& context);

  // Creates a new span
  trpc::opentelemetry::OpenTelemetryTracingSpanPtr NewSpan(const ClientContextPtr& context,
                                                           const std::any& parent_span);

  // Finishes the span
  void FinishSpan(const std::any& any_span, const ClientContextPtr& context);

  // Adds an event containing request data to span
  void AddRequestEvent(const ClientContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

  // Adds an event containing response data to span
  void AddRsponseEvent(const ClientContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

  /// @brief Determines whether to add an event containing request/response data to span.
  bool NeedReportReqRsp(const ClientContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

 protected:
  OpenTelemetryTracingPtr tracer_factory_;

  bool disable_trace_body_ = true;
  bool deferred_sample_error_ = false;
  bool use_grpc_reported_ = false;
};

}  // namespace trpc
