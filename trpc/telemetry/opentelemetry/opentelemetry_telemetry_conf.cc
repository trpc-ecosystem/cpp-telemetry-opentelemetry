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

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"
#include "trpc/util/log/logging.h"

namespace trpc {

void OpenTelemetrySamplerConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("fraction: {}", fraction);

  TRPC_LOG_DEBUG("");
}

void OpenTelemetryMetricsCode::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("code: {}", code);
  TRPC_FMT_DEBUG("type: {}", type);
  TRPC_FMT_DEBUG("description {}", description);
  TRPC_FMT_DEBUG("service: {}", service);
  TRPC_FMT_DEBUG("method: {}", method);

  TRPC_LOG_DEBUG("");
}

void OpenTelemetryMetricsConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("enabled: {}", enabled);

  TRPC_LOG_DEBUG("client_histogram_buckets:");
  for (size_t i = 0; i < client_histogram_buckets.size(); i++) {
    TRPC_FMT_DEBUG("{} : {}", i, client_histogram_buckets[i]);
  }

  TRPC_LOG_DEBUG("server_histogram_buckets:");
  for (size_t i = 0; i < server_histogram_buckets.size(); i++) {
    TRPC_FMT_DEBUG("{} : {}", i, server_histogram_buckets[i]);
  }

  TRPC_LOG_DEBUG("codes:");
  for (auto code : codes) {
    code.Display();
  }

  TRPC_LOG_DEBUG("");
}

void OpenTelemetryLogsConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("enabled: {}", enabled);
  TRPC_FMT_DEBUG("level: {}", level);
  TRPC_FMT_DEBUG("enable_sampler: {}", enable_sampler);
  TRPC_FMT_DEBUG("enable_sampler_error: {}", enable_sampler_error);
  TRPC_LOG_DEBUG("resources:");
  for (auto resource : resources) {
    TRPC_LOG_DEBUG(resource.first << ":" << resource.second);
  }

  TRPC_LOG_DEBUG("");
}

void OpenTelemetryTracesConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("disable_trace_body: {}", disable_trace_body);
  TRPC_FMT_DEBUG("enable_deferred_sample: {}", enable_deferred_sample);
  TRPC_FMT_DEBUG("deferred_sample_error: {}", deferred_sample_error);
  TRPC_FMT_DEBUG("deferred_sample_slow_duration: {}", deferred_sample_slow_duration);
  TRPC_FMT_DEBUG("disable_parent_sampling: {}", disable_parent_sampling);
  TRPC_LOG_DEBUG("resources:");
  for (auto resource : resources) {
    TRPC_LOG_DEBUG(resource.first << ":" << resource.second);
  }

  TRPC_LOG_DEBUG("");
}

void OpenTelemetryConfig::Display() const {
  TRPC_LOG_DEBUG("--------------------------------");

  TRPC_FMT_DEBUG("addr: {}", addr);
  TRPC_FMT_DEBUG("protocol: {}", protocol);
  TRPC_FMT_DEBUG("selector_name: {}", selector_name);
  TRPC_FMT_DEBUG("timeout: {}", timeout);

  sampler_config.Display();
  metrics_config.Display();
  logs_config.Display();
  traces_config.Display();

  TRPC_LOG_DEBUG("");
}

}  // namespace trpc
