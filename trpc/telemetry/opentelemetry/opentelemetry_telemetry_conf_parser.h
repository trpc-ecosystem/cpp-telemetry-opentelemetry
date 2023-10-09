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

#include "yaml-cpp/yaml.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"

namespace YAML {

template <>
struct convert<trpc::OpenTelemetryConfig> {
  static YAML::Node encode(const trpc::OpenTelemetryConfig& config) {
    YAML::Node node;

    node["addr"] = config.addr;

    node["protocol"] = config.protocol;

    node["selector_name"] = config.selector_name;

    node["timeout"] = config.timeout;

    node["sampler"] = config.sampler_config;

    node["metrics"] = config.metrics_config;

    node["logs"] = config.logs_config;

    node["traces"] = config.traces_config;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetryConfig& config) {
    if (node["addr"]) {
      config.addr = node["addr"].as<std::string>();
    }

    if (node["protocol"]) {
      config.protocol = node["protocol"].as<std::string>();
    }

    if (node["selector_name"]) {
      config.selector_name = node["selector_name"].as<std::string>();
    }

    if (node["timeout"]) {
      config.timeout = node["timeout"].as<int>();
    }

    if (node["sampler"]) {
      config.sampler_config = node["sampler"].as<trpc::OpenTelemetrySamplerConfig>();
    }

    if (node["metrics"]) {
      config.metrics_config = node["metrics"].as<trpc::OpenTelemetryMetricsConfig>();
    }

    if (node["logs"]) {
      config.logs_config = node["logs"].as<trpc::OpenTelemetryLogsConfig>();
    }

    if (node["traces"]) {
      config.traces_config = node["traces"].as<trpc::OpenTelemetryTracesConfig>();
    }

    return true;
  }
};

template <>
struct convert<trpc::OpenTelemetrySamplerConfig> {
  static YAML::Node encode(const trpc::OpenTelemetrySamplerConfig& config) {
    YAML::Node node;

    node["fraction"] = config.fraction;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetrySamplerConfig& config) {
    if (node["fraction"]) {
      config.fraction = node["fraction"].as<double>();
    }

    return true;
  }
};

template <>
struct convert<trpc::OpenTelemetryMetricsCode> {
  static YAML::Node encode(const trpc::OpenTelemetryMetricsCode& config) {
    YAML::Node node;

    node["code"] = config.code;
    node["type"] = config.type;
    node["description"] = config.description;
    node["service"] = config.service;
    node["method"] = config.method;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetryMetricsCode& config) {
    if (node["code"]) {
      config.code = node["code"].as<uint64_t>();
    }
    if (node["type"]) {
      config.type = node["type"].as<std::string>();
    }
    if (node["description"]) {
      config.description = node["description"].as<std::string>();
    }
    if (node["service"]) {
      config.service = node["service"].as<std::string>();
    }
    if (node["method"]) {
      config.method = node["method"].as<std::string>();
    }

    return true;
  }
};

template <>
struct convert<trpc::OpenTelemetryMetricsConfig> {
  static YAML::Node encode(const trpc::OpenTelemetryMetricsConfig& config) {
    YAML::Node node;

    node["enabled"] = config.enabled;
    node["client_histogram_buckets"] = config.client_histogram_buckets;
    node["server_histogram_buckets"] = config.server_histogram_buckets;
    node["codes"] = config.codes;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetryMetricsConfig& config) {
    if (node["enabled"]) {
      config.enabled = node["enabled"].as<bool>();
    }
    if (node["codes"]) {
      config.codes = node["codes"].as<std::vector<trpc::OpenTelemetryMetricsCode>>();
    }

    if (node["client_histogram_buckets"]) {
      config.client_histogram_buckets = node["client_histogram_buckets"].as<std::vector<double>>();
    }

    if (node["server_histogram_buckets"]) {
      config.server_histogram_buckets = node["server_histogram_buckets"].as<std::vector<double>>();
    }

    return true;
  }
};

template <>
struct convert<trpc::OpenTelemetryLogsConfig> {
  static YAML::Node encode(const trpc::OpenTelemetryLogsConfig& config) {
    YAML::Node node;

    node["enabled"] = config.enabled;

    node["level"] = config.level;

    node["enable_sampler"] = config.enable_sampler;

    node["enable_sampler_error"] = config.enable_sampler_error;

    node["resources"] = config.resources;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetryLogsConfig& config) {
    if (node["enabled"]) {
      config.enabled = node["enabled"].as<bool>();
    }

    if (node["level"]) {
      config.level = node["level"].as<std::string>();
    }

    if (node["enable_sampler"]) {
      config.enable_sampler = node["enable_sampler"].as<bool>();
    }

    if (node["enable_sampler_error"]) {
      config.enable_sampler_error = node["enable_sampler_error"].as<bool>();
    }

    if (node["resources"]) {
      config.resources = node["resources"].as<std::map<std::string, std::string>>();
    }

    return true;
  }
};

template <>
struct convert<trpc::OpenTelemetryTracesConfig> {
  static YAML::Node encode(const trpc::OpenTelemetryTracesConfig& config) {
    YAML::Node node;

    node["disable_trace_body"] = config.disable_trace_body;

    node["enable_deferred_sample"] = config.enable_deferred_sample;

    node["deferred_sample_error"] = config.deferred_sample_error;

    node["deferred_sample_slow_duration"] = config.deferred_sample_slow_duration;

    node["disable_parent_sampling"] = config.disable_parent_sampling;

    node["resources"] = config.resources;

    return node;
  }

  static bool decode(const YAML::Node& node, trpc::OpenTelemetryTracesConfig& config) {
    if (node["disable_trace_body"]) {
      config.disable_trace_body = node["disable_trace_body"].as<bool>();
    }

    if (node["enable_deferred_sample"]) {
      config.enable_deferred_sample = node["enable_deferred_sample"].as<bool>();
    }

    if (node["deferred_sample_error"]) {
      config.deferred_sample_error = node["deferred_sample_error"].as<bool>();
    }

    if (node["deferred_sample_slow_duration"]) {
      config.deferred_sample_slow_duration = node["deferred_sample_slow_duration"].as<int>();
    }

    if (node["disable_parent_sampling"]) {
      config.disable_parent_sampling = node["disable_parent_sampling"].as<bool>();
    }

    if (node["resources"]) {
      config.resources = node["resources"].as<std::map<std::string, std::string>>();
    }

    return true;
  }
};

}  // namespace YAML
