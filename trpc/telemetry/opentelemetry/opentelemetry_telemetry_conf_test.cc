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
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"

#include "gtest/gtest.h"
#include "yaml-cpp/yaml.h"

namespace trpc::testing {

TEST(OpenTelemetryConfigTest, ConfCodec) {
  OpenTelemetryConfig config;
  config.addr = "127.0.0.1:8888";
  config.protocol = "http";
  config.selector_name = "direct";
  config.timeout = 1000;

  config.sampler_config.fraction = 0.001;

  config.metrics_config.enabled = true;
  config.metrics_config.client_histogram_buckets = {1, 2, 3, 4};
  config.metrics_config.server_histogram_buckets = {1, 2, 3, 4};
  OpenTelemetryMetricsCode metric_code;
  metric_code.code = 0;
  metric_code.type = "success";
  metric_code.description = "success";
  metric_code.service = "service";
  metric_code.method = "method";
  config.metrics_config.codes.push_back(metric_code);

  config.logs_config.enabled = true;
  config.logs_config.level = "info";
  config.logs_config.enable_sampler = true;
  config.logs_config.enable_sampler_error = true;
  config.logs_config.resources["tenant.id"] = "default";

  config.traces_config.disable_trace_body = true;
  config.traces_config.enable_deferred_sample = false;
  config.traces_config.deferred_sample_error = false;
  config.traces_config.deferred_sample_slow_duration = 10000;
  config.traces_config.disable_parent_sampling = false;
  config.traces_config.resources["tenant.id"] = "default";

  config.Display();

  auto node = YAML::convert<trpc::OpenTelemetryConfig>::encode(config);

  OpenTelemetryConfig copy_config;
  ASSERT_TRUE(YAML::convert<trpc::OpenTelemetryConfig>::decode(node, copy_config));

  ASSERT_EQ(config.addr, copy_config.addr);
  ASSERT_EQ(config.protocol, copy_config.protocol);
  ASSERT_EQ(config.selector_name, copy_config.selector_name);
  ASSERT_EQ(config.timeout, copy_config.timeout);

  ASSERT_EQ(config.sampler_config.fraction, copy_config.sampler_config.fraction);

  ASSERT_EQ(config.metrics_config.enabled, copy_config.metrics_config.enabled);
  ASSERT_EQ(config.metrics_config.codes.size(), copy_config.metrics_config.codes.size());
  ASSERT_EQ(config.metrics_config.client_histogram_buckets.size(),
            copy_config.metrics_config.client_histogram_buckets.size());
  ASSERT_EQ(config.metrics_config.server_histogram_buckets.size(),
            copy_config.metrics_config.server_histogram_buckets.size());

  ASSERT_EQ(config.logs_config.enabled, copy_config.logs_config.enabled);
  ASSERT_EQ(config.logs_config.level, copy_config.logs_config.level);
  ASSERT_EQ(config.logs_config.enable_sampler, copy_config.logs_config.enable_sampler);
  ASSERT_EQ(config.logs_config.enable_sampler_error, copy_config.logs_config.enable_sampler_error);
  ASSERT_EQ(config.logs_config.resources, copy_config.logs_config.resources);

  ASSERT_EQ(config.traces_config.disable_trace_body, copy_config.traces_config.disable_trace_body);
  ASSERT_EQ(config.traces_config.enable_deferred_sample, copy_config.traces_config.enable_deferred_sample);
  ASSERT_EQ(config.traces_config.deferred_sample_error, copy_config.traces_config.deferred_sample_error);
  ASSERT_EQ(config.traces_config.deferred_sample_slow_duration,
            copy_config.traces_config.deferred_sample_slow_duration);
  ASSERT_EQ(config.traces_config.disable_parent_sampling, copy_config.traces_config.disable_parent_sampling);
  ASSERT_EQ(config.traces_config.resources, copy_config.traces_config.resources);
}

}  // namespace trpc::testing
