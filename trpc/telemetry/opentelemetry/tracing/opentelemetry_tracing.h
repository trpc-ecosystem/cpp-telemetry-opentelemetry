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

#include "opentelemetry/sdk/trace/exporter.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"
#include "trpc/tracing/tracing.h"

namespace trpc {

/// @brief The implementation class for tracing capabilities of the OpenTelemetry plugin.
class OpenTelemetryTracing : public Tracing {
 public:
  int Init() noexcept override;

  std::string Name() const override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  /// @brief Gets a tracer with the requested service name
  /// @param service_name used to identify the tracer
  /// @param error_message used to record error information when getting fails
  /// @return the corresponding tracer.
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> MakeTracer(const char* service_name,
                                                                                std::string& error_message);

  /// @brief Gets the config for OpenTelemetryTracing
  const OpenTelemetryConfig& GetConfig() { return config_; }

 private:
  std::unique_ptr<::opentelemetry::sdk::trace::SpanExporter> GetExporter();

  bool InitOpenTelemetry();

 private:
  OpenTelemetryConfig config_;
};

using OpenTelemetryTracingPtr = RefPtr<OpenTelemetryTracing>;

}  // namespace trpc
