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

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/util/log/default/default_log.h"
#include "trpc/util/log/log.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_log_handler.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"

namespace trpc {

OpenTelemetryTelemetry::OpenTelemetryTelemetry() {
  tracing_ = MakeRefCounted<OpenTelemetryTracing>();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_ = MakeRefCounted<OpenTelemetryMetrics>();
#endif
#ifdef ENABLE_LOGS_PREVIEW
  log_ = MakeRefCounted<OpenTelemetryLogging>();
#endif
}

int OpenTelemetryTelemetry::Init() noexcept {
  bool ret = TrpcConfig::GetInstance()->GetPluginConfig("telemetry", trpc::opentelemetry::kOpenTelemetryTelemetryName,
                                                        config_);
  if (!ret) {
    TRPC_FMT_ERROR("get {} config fail", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }

  // sets the callback for logging in the OpenTelemetry library, which needs to be set before creating the Provider.
  std::shared_ptr<::opentelemetry::sdk::common::internal_log::LogHandler> log_handler =
      std::make_shared<trpc::opentelemetry::LogHandler>();
  ::opentelemetry::sdk::common::internal_log::GlobalLogHandler::SetLogHandler(log_handler);

  // initializes tracing
  int tracing_ret = tracing_->Init();
  if (tracing_ret != 0) {
    TRPC_LOG_ERROR("Init of tracing fail! ret:" << tracing_ret);
    return -1;
  }

  // initializes metrics
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  int metrics_ret = metrics_->Init();
  if (metrics_ret != 0) {
    TRPC_LOG_ERROR("Init of metrics fail! ret:" << metrics_ret);
    return -1;
  }
#endif

  // initializes logging
#ifdef ENABLE_LOGS_PREVIEW
  if (InitLog() != 0) {
    return -1;
  }
#endif

  return 0;
}

int OpenTelemetryTelemetry::InitLog() {
#ifdef ENABLE_LOGS_PREVIEW
  const auto& p = LogFactory::GetInstance()->Get();
  if (p) {
    if (config_.logs_config.enabled) {
      int log_ret = log_->Init();
      if (log_ret != 0) {
        TRPC_LOG_ERROR("Init of logging fail! ret:" << log_ret);
        return -1;
      }

      DefaultLog::Logger logger;
      logger.config.min_level = GetLogLevel(config_.logs_config.level);
      logger.logger = nullptr;
      logger.raw_sinks = {log_};
      static_pointer_cast<DefaultLog>(p)->RegisterInstance(trpc::opentelemetry::kOpenTelemetryLoggerName, logger);
    } else {  // 注册一个空的tpstelemetry_log instance，避免关闭enabled后DefaultLog打印找不到instance错误
      DefaultLog::Logger logger;
      logger.config.min_level = Log::Level::critical;
      logger.logger = nullptr;
      logger.raw_sinks = {};
      static_pointer_cast<DefaultLog>(p)->RegisterInstance(trpc::opentelemetry::kOpenTelemetryLoggerName, logger);
    }
  } else {
    TRPC_LOG_ERROR("LogFactory is empty");
    return -1;
  }
#endif
  return 0;
}

Log::Level OpenTelemetryTelemetry::GetLogLevel(const std::string& level) {
  std::unordered_map<std::string, Log::Level> level_map;
  level_map["trace"] = trpc::Log::trace;
  level_map["debug"] = trpc::Log::debug;
  level_map["info"] = trpc::Log::info;
  level_map["warn"] = trpc::Log::warn;
  level_map["error"] = trpc::Log::error;
  level_map["fatal"] = trpc::Log::critical;

  if (level_map.find(level) != level_map.end()) {
    return level_map[level];
  }
  return trpc::Log::error;
}

void OpenTelemetryTelemetry::Start() noexcept {
  tracing_->Start();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_->Start();
#endif
#ifdef ENABLE_LOGS_PREVIEW
  log_->Start();
#endif
}

void OpenTelemetryTelemetry::Stop() noexcept {
  tracing_->Stop();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_->Stop();
#endif
#ifdef ENABLE_LOGS_PREVIEW
  log_->Stop();
#endif
}

void OpenTelemetryTelemetry::Destroy() noexcept {
  tracing_->Destroy();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_->Destroy();
#endif
#ifdef ENABLE_LOGS_PREVIEW
  log_->Destroy();
#endif
}

TracingPtr OpenTelemetryTelemetry::GetTracing() { return tracing_; }

MetricsPtr OpenTelemetryTelemetry::GetMetrics() {
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  return metrics_;
#else
  return nullptr;
#endif
}

LoggingPtr OpenTelemetryTelemetry::GetLog() {
#ifdef ENABLE_LOGS_PREVIEW
  return log_;
#else
  return nullptr;
#endif
}

}  // namespace trpc
