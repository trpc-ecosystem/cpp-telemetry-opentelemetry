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

#ifdef ENABLE_LOGS_PREVIEW
#pragma once

#include <unordered_map>

#include "opentelemetry/logs/logger.h"
#include "opentelemetry/sdk/logs/exporter.h"
#include "trpc/log/logging.h"
#include "trpc/tracing/tracing_filter_index.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"

namespace trpc {

/// @brief The implementation class for logging capabilities of the OpenTelemetry plugin.
class OpenTelemetryLogging : public Logging {
 public:
  std::string Name() const override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  std::string LoggerName() const override { return trpc::opentelemetry::kOpenTelemetryLoggerName; }

  int Init() noexcept override;

  void Log(const Log::Level level, const char* filename_in, int line_in, const char* funcname_in, std::string_view msg,
           const std::unordered_map<uint32_t, std::any>& extend_fields_msg) override;

 private:
  std::unique_ptr<::opentelemetry::sdk::logs::LogRecordExporter> GetExporter();

  bool InitOpenTelemetry();

  // Gets the ServerTracingSpan from extend_fields_msg
  const ServerTracingSpan* GetServerTracingSpan(const std::unordered_map<uint32_t, std::any>& extend_fields_msg);

  // Initializes level_serverity_mapping_
  void InitLogLevelMapping();

  // Gets the opentelemetry log level
  ::opentelemetry::logs::Severity GetOpenTelemetryLogLevel(Log::Level trpc_log_level);

  // Checks if reporting is necessary
  bool ShouldReport(Log::Level level, bool is_sample);

 private:
  OpenTelemetryConfig config_;

  // a map of framework log levels to opentelemetry log levels.
  std::unordered_map<Log::Level, ::opentelemetry::logs::Severity> level_serverity_mapping_;

  // opentelemetry logger
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::logs::Logger> logger_;
};

using OpenTelemetryLoggingPtr = RefPtr<OpenTelemetryLogging>;

}  // namespace trpc
#endif
