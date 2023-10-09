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

#include "trpc/telemetry/telemetry.h"

#ifdef ENABLE_LOGS_PREVIEW
#include "trpc/telemetry/opentelemetry/logging/opentelemetry_logging.h"
#endif
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"
#endif
#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"

namespace trpc {

class OpenTelemetryTelemetry : public Telemetry {
 public:
  OpenTelemetryTelemetry();

  std::string Name() const override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  int Init() noexcept override;

  void Start() noexcept override;

  void Stop() noexcept override;

  void Destroy() noexcept override;

  TracingPtr GetTracing() override;

  MetricsPtr GetMetrics() override;

  LoggingPtr GetLog() override;

 private:
  // Initializes logging
  int InitLog();

  Log::Level GetLogLevel(const std::string& level);

 private:
  OpenTelemetryConfig config_;

  OpenTelemetryTracingPtr tracing_ = nullptr;

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  OpenTelemetryMetricsPtr metrics_ = nullptr;
#endif

#ifdef ENABLE_LOGS_PREVIEW
  OpenTelemetryLoggingPtr log_ = nullptr;
#endif
};

using OpenTelemetryTelemetryPtr = RefPtr<OpenTelemetryTelemetry>;

}  // namespace trpc
