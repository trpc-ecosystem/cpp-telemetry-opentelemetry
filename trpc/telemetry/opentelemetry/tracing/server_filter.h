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

#include <any>
#include <string>
#include <vector>

#include "trpc/filter/filter.h"
#include "trpc/server/server_context.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"
#include "trpc/telemetry/opentelemetry/tracing/text_map_carrier.h"

namespace trpc {

namespace opentelemetry {

/// @brief The server-side TextMapCarrier retrieval function which returns a specific implementation of TextMapCarrier
///        based on the ServerContext.
/// @param context ServerContext
/// @return a specific implementation of TextMapCarrier
using ServerTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ServerContextPtr& context)>;

/// @brief Sets a server-side TextMapCarrier retrieval function for a specific protocol.
/// @param protocol_name protocol name
/// @param carrier_func TextMapCarrier retrieval function
void SetServerCarrierFunc(const std::string& protocol_name, const ServerTextMapCarrierFunc& carrier_func);

/// @brief Gets a server-side TextMapCarrier retrieval function for a specific protocol.
/// @param protocol_name protocol name
/// @return TextMapCarrier retrieval function
ServerTextMapCarrierFunc GetServerCarrierFunc(const std::string& protocol_name);

/// @brief The implementation function for constructing a TextMapCarrier using transinfo.
TextMapCarrierPtr ServerTransInfoCarrierFunc(const ServerContextPtr& context);

/// @brief The implementation function for constructing a TextMapCarrier using HttpRequest.
TextMapCarrierPtr ServerHttpCarrierFunc(const ServerContextPtr& context);

}  // namespace opentelemetry

class OpenTelemetryTracingServerFilter : public trpc::MessageServerFilter {
 public:
  int Init() override;

  std::string Name() override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  std::vector<FilterPoint> GetFilterPoint() override;

  void operator()(FilterStatus& status, FilterPoint point, const ServerContextPtr& context) override;

 private:
  // Gets a tracer from tracer_factory_
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> GetTracer(const ServerContextPtr& context);

  // Creates a new span
  trpc::opentelemetry::OpenTelemetryTracingSpanPtr NewSpan(const ServerContextPtr& context);

  // Finishes the span
  void FinishSpan(const std::any& any_span, const ServerContextPtr& context);

  // Adds an event containing request data to span
  void AddRequestEvent(const ServerContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

  // Adds an event containing response data to span
  void AddRsponseEvent(const ServerContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

  /// @brief Determines whether to add an event containing request/response data to span.
  bool NeedReportReqRsp(const ServerContextPtr& context, const trpc::opentelemetry::OpenTelemetryTracingSpanPtr& span);

 protected:
  OpenTelemetryTracingPtr tracer_factory_ = nullptr;

  bool disable_trace_body_ = true;
  bool deferred_sample_error_ = false;
};

}  // namespace trpc
