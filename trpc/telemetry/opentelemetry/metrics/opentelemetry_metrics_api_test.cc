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

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics_api.h"

#include "gtest/gtest.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"
#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"

namespace trpc::testing {

class OpenTelemetryMetricsAPITest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    metrics_ = MakeRefCounted<OpenTelemetryMetrics>();
    ASSERT_EQ(0, metrics_->Init());
  }
  static void TearDownTestCase() {}

 protected:
  static OpenTelemetryMetricsPtr metrics_;
};

OpenTelemetryMetricsPtr OpenTelemetryMetricsAPITest::metrics_;

std::map<std::string, std::string> GetTestLabels(std::string value) {
  std::map<std::string, std::string> labels = {{"inter_key1", value + "1"}, {"inter_key2", value + "2"}};
  return labels;
}

TEST_F(OpenTelemetryMetricsAPITest, Report) {
  // 1. testing report when opentelemetry plugin is not registered
  std::map<std::string, std::string> labels = GetTestLabels("inter_value");
  ASSERT_NE(0, trpc::opentelemetry::ReportSetMetricsInfo(labels, 10));

  // registers a plugin to enable successful invocation of user reporting interface.
  auto telemetry = MakeRefCounted<MockOpenTelemetryTelemetry>();
  TelemetryFactory::GetInstance()->Register(telemetry);
  EXPECT_CALL(*telemetry, GetMetrics()).WillRepeatedly(::testing::Return(metrics_));

  // 2. testing report SET metrics data
  labels = GetTestLabels("inter_set_value");
  ASSERT_EQ(0, trpc::opentelemetry::ReportSetMetricsInfo(labels, 10));

  // 3. testing report SUM metrics data
  labels = GetTestLabels("inter_sum_value");
  ASSERT_EQ(0, trpc::opentelemetry::ReportSumMetricsInfo(labels, 10));

  // 4. testing report MID metrics data
  labels = GetTestLabels("inter_mid_value");
  ASSERT_EQ(0, trpc::opentelemetry::ReportMidMetricsInfo(labels, 10));

  // 5. testing report QUANTILES metrics data
  labels = GetTestLabels("inter_quantiles_value");
  // report failed because quantiles filed is empty
  ASSERT_NE(0, trpc::opentelemetry::ReportQuantilesMetricsInfo(labels, {}, 10));
  // report failed because the value in quantiles do not have a size of 2
  ASSERT_NE(0, trpc::opentelemetry::ReportQuantilesMetricsInfo(labels, {{0.5}}, 10));
  // report success because the metrics data is valid
  ASSERT_EQ(0, trpc::opentelemetry::ReportQuantilesMetricsInfo(labels, {{0.5, 0.05}, {0.1, 0.05}}, 10));

  // 6. testing report HISTOGRAM metrics data
  labels = GetTestLabels("inter_histogram_value");
  // report failed because bucket filed is empty
  ASSERT_NE(0, trpc::opentelemetry::ReportHistogramMetricsInfo(labels, {}, 10));
  // report success because the metrics data is valid
  trpc::HistogramBucket bucket = {0.1, 0.5, 1};
  ASSERT_EQ(0, trpc::opentelemetry::ReportHistogramMetricsInfo(labels, bucket, 10));
  ASSERT_EQ(0, trpc::opentelemetry::ReportHistogramMetricsInfo(labels, std::move(bucket), 10));
}

}  // namespace trpc::testing
#endif
