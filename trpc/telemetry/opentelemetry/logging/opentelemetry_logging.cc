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
#include "trpc/telemetry/opentelemetry/logging/opentelemetry_logging.h"

#include "opentelemetry/exporters/otlp/otlp_http_log_record_exporter.h"
#include "opentelemetry/logs/provider.h"
#include "opentelemetry/sdk/logs/batch_log_record_processor.h"
#include "opentelemetry/sdk/logs/logger_provider_factory.h"
#include "trpc/common/config/trpc_config.h"

#include "trpc/telemetry/opentelemetry/logging/common.h"
#include "trpc/telemetry/opentelemetry/logging/grpc_log_exporter.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing_api.h"

namespace trpc {

int OpenTelemetryLogging::Init() noexcept {
  bool ret = TrpcConfig::GetInstance()->GetPluginConfig("telemetry", trpc::opentelemetry::kOpenTelemetryTelemetryName,
                                                        config_);
  if (!ret) {
    TRPC_FMT_ERROR("get {} config fail", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }

  if (!InitOpenTelemetry()) {
    TRPC_LOG_ERROR("InitOpenTelemetry failed...");
    return -1;
  }

  InitLogLevelMapping();

  return 0;
}

bool OpenTelemetryLogging::InitOpenTelemetry() {
  // initializes exporter
  auto exporter = GetExporter();
  if (!exporter) {
    TRPC_FMT_ERROR("get opentelemetry exporter fail, protocol is invalid: {}", config_.protocol);
    return false;
  }

  // initializes processor
  auto processor = std::make_unique<::opentelemetry::sdk::logs::BatchLogRecordProcessor>(std::move(exporter));

  // initializes provider
  const auto& global_config = trpc::TrpcConfig::GetInstance()->GetGlobalConfig();
  const auto& server_config = trpc::TrpcConfig::GetInstance()->GetServerConfig();
  std::string server = server_config.app + "." + server_config.server;
  std::string env = global_config.env_name;
  ::opentelemetry::sdk::common::AttributeMap resources_map = {{trpc::opentelemetry::kLogServerName, server},
                                                              {trpc::opentelemetry::kLogServiceName, server},
                                                              {trpc::opentelemetry::kLogEnvName, env}};
  for (auto& [key, value] : config_.traces_config.resources) {
    resources_map.SetAttribute(key, value);
  }
  auto resource = ::opentelemetry::sdk::resource::Resource::Create(resources_map);
  auto provider = ::opentelemetry::sdk::logs::LoggerProviderFactory::Create(std::move(processor), resource);
  ::opentelemetry::logs::Provider::SetLoggerProvider(
      ::opentelemetry::nostd::shared_ptr<::opentelemetry::logs::LoggerProvider>(provider.release()));

  // initializes logger
  logger_ = ::opentelemetry::logs::Provider::GetLoggerProvider()->GetLogger(
      "opentelemetry_logger", "", "opentelelemtry_library", "", "https://opentelemetry.io/schemas/1.11.0");
  return true;
}

std::unique_ptr<::opentelemetry::sdk::logs::LogRecordExporter> OpenTelemetryLogging::GetExporter() {
  if (config_.protocol == "http") {
    ::opentelemetry::exporter::otlp::OtlpHttpLogRecordExporterOptions logger_opts;
    logger_opts.url = config_.addr + "/v1/logs";
    return std::make_unique<::opentelemetry::exporter::otlp::OtlpHttpLogRecordExporter>(logger_opts);
  } else if (config_.protocol == "grpc") {
    ServiceProxyOption service_opts;
    service_opts.name = trpc::opentelemetry::kGrpcLogExporterServiceName;
    service_opts.codec_name = config_.protocol;
    service_opts.selector_name = config_.selector_name;
    service_opts.timeout = config_.timeout;
    service_opts.target = config_.addr;
    return std::make_unique<trpc::opentelemetry::GrpcLogExporter>(service_opts);
  }
  return nullptr;
}

void OpenTelemetryLogging::InitLogLevelMapping() {
  level_serverity_mapping_[Log::Level::trace] = ::opentelemetry::logs::Severity::kTrace;
  level_serverity_mapping_[Log::Level::debug] = ::opentelemetry::logs::Severity::kDebug;
  level_serverity_mapping_[Log::Level::info] = ::opentelemetry::logs::Severity::kInfo;
  level_serverity_mapping_[Log::Level::warn] = ::opentelemetry::logs::Severity::kWarn;
  level_serverity_mapping_[Log::Level::error] = ::opentelemetry::logs::Severity::kError;
  level_serverity_mapping_[Log::Level::critical] = ::opentelemetry::logs::Severity::kFatal;
}

void OpenTelemetryLogging::Log(const Log::Level level, const char* filename_in, int line_in, const char* funcname_in,
                               std::string_view msg, const std::unordered_map<uint32_t, std::any>& extend_fields_msg) {
  auto* server_tracing_span = GetServerTracingSpan(extend_fields_msg);
  if (!server_tracing_span) {
    return;
  }

  if (server_tracing_span->span.type() != typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr) ||
      std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(server_tracing_span->span).get() ==
          nullptr) {
    return;
  }
  const auto span = std::any_cast<const trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(server_tracing_span->span);

  auto span_ctx = span->GetContext();
  if (ShouldReport(level, span_ctx.IsSampled())) {
    std::string line = std::string(filename_in) + ":" + std::to_string(line_in);
    logger_->EmitLogRecord(GetOpenTelemetryLogLevel(level), msg.data(),
                           ::opentelemetry::common::MakeAttributes({{"line", std::move(line)}}), span_ctx.trace_id(),
                           span_ctx.span_id(), span_ctx.trace_flags(), std::chrono::system_clock::now());
  }
}

const ServerTracingSpan* OpenTelemetryLogging::GetServerTracingSpan(
    const std::unordered_map<uint32_t, std::any>& extend_fields_msg) {
  uint32_t filter_index = trpc::opentelemetry::GetTracingFilterDataIndex();
  if (filter_index == trpc::opentelemetry::kInvalidTracingFilterDataIndex) {
    return nullptr;
  }
  auto it = extend_fields_msg.find(filter_index);
  if (it == extend_fields_msg.end()) {
    return nullptr;
  }
  return std::any_cast<ServerTracingSpan>(&(it->second));
}

::opentelemetry::logs::Severity OpenTelemetryLogging::GetOpenTelemetryLogLevel(Log::Level trpc_log_level) {
  if (level_serverity_mapping_.find(trpc_log_level) != level_serverity_mapping_.end()) {
    return level_serverity_mapping_[trpc_log_level];
  }
  return ::opentelemetry::logs::Severity::kError;
}

bool OpenTelemetryLogging::ShouldReport(Log::Level level, bool is_sample) {
  if (config_.logs_config.enable_sampler) {  // only reports the sampled logs
    if (is_sample) {                         // sampled
      return true;
    }
    if (config_.logs_config.enable_sampler_error &&
        level >= trpc::Log::error) {  // not sampled, but log level is error or higher.
      return true;
    }
    return false;
  }
  // reports all logs
  return true;
}

}  // namespace trpc
#endif
