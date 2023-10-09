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

#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing_api.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/tracing/tracing_filter_index.h"
#include "trpc/util/log/logging.h"

namespace trpc::opentelemetry {

namespace {

OpenTelemetryTracingPtr GetTracingPlugin() {
  TelemetryPtr telemetry = TelemetryFactory::GetInstance()->Get(trpc::opentelemetry::kOpenTelemetryTelemetryName);
  if (!telemetry) {
    TRPC_FMT_DEBUG("get telemetry plugin {} failed.", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return nullptr;
  }
  OpenTelemetryTracingPtr tracing = trpc::dynamic_pointer_cast<trpc::OpenTelemetryTracing>(telemetry->GetTracing());
  return tracing;
}

}  // namespace

uint32_t GetTracingFilterDataIndex() {
  auto tracing = GetTracingPlugin();
  if (tracing) {
    return tracing->GetPluginID();
  }
  return kInvalidTracingFilterDataIndex;
}

OpenTelemetryTracingSpanPtr GetTracingSpan(const ServerContextPtr& context) {
  uint32_t filter_index = GetTracingFilterDataIndex();
  if (filter_index != kInvalidTracingFilterDataIndex) {
    auto* server_tracing_span = context->GetFilterData<ServerTracingSpan>(filter_index);
    if (server_tracing_span) {
      auto* tracing_span = std::any_cast<OpenTelemetryTracingSpanPtr>(&server_tracing_span->span);
      if (tracing_span) {
        return *tracing_span;
      }
    }
  }
  return OpenTelemetryTracingSpanPtr(nullptr);
}

std::string GetTraceID(const ServerContextPtr& context) {
  auto span = GetTracingSpan(context);
  if (span.get() != nullptr) {
    char trace_id[32];
    span->GetContext().trace_id().ToLowerBase16(trace_id);
    return std::string(trace_id, sizeof(trace_id));
  }
  return "";
}

std::string GetSpanID(const ServerContextPtr& context) {
  auto span = GetTracingSpan(context);
  if (span.get() != nullptr) {
    char span_id[16];
    span->GetContext().span_id().ToLowerBase16(span_id);
    return std::string(span_id, sizeof(span_id));
  }
  return "";
}

}  // namespace trpc::opentelemetry
