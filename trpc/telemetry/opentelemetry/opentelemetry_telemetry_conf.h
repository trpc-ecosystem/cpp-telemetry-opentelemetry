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

#include <map>
#include <string>
#include <vector>

#include "yaml-cpp/yaml.h"

namespace trpc {

struct OpenTelemetrySamplerConfig {
  double fraction = 1;

  void Display() const;
};

struct OpenTelemetryMetricsCode {
  uint64_t code;
  std::string type;
  std::string description;
  std::string service;
  std::string method;

  void Display() const;
};

struct OpenTelemetryMetricsConfig {
  bool enabled = false;
  std::vector<double> client_histogram_buckets = {0.005, 0.01, 0.1, 0.5, 1, 5};
  std::vector<double> server_histogram_buckets = {0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 5};
  std::vector<OpenTelemetryMetricsCode> codes;

  void Display() const;
};

struct OpenTelemetryLogsConfig {
  bool enabled = false;
  std::string level = "error";
  bool enable_sampler = false;
  bool enable_sampler_error = false;
  std::map<std::string, std::string> resources;

  void Display() const;
};

struct OpenTelemetryTracesConfig {
  bool disable_trace_body = true;
  bool enable_deferred_sample = false;
  bool deferred_sample_error = false;
  /// The unit of timeout is milliseconds
  int deferred_sample_slow_duration = 500;
  bool disable_parent_sampling = false;
  std::map<std::string, std::string> resources;

  void Display() const;
};

/// @brief Configuration of OpenTelemetry telemetry plugin.
struct OpenTelemetryConfig {
  std::string addr;
  std::string protocol = "http";
  std::string selector_name = "domain";
  /// The unit of timeout is milliseconds
  int timeout = 10000;
  OpenTelemetrySamplerConfig sampler_config;
  OpenTelemetryMetricsConfig metrics_config;
  OpenTelemetryLogsConfig logs_config;
  OpenTelemetryTracesConfig traces_config;

  void Display() const;
};

}  // namespace trpc
