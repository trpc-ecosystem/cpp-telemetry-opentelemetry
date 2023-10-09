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
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"

#include "gtest/gtest.h"
#include "trpc/common/config/trpc_config.h"

#include "trpc/telemetry/opentelemetry/metrics/common.h"

namespace trpc::testing {

class OpenTelemetryMetricsTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { metrics_ = MakeRefCounted<OpenTelemetryMetrics>(); }
  static void TearDownTestCase() {}

 protected:
  static OpenTelemetryMetricsPtr metrics_;
};

OpenTelemetryMetricsPtr OpenTelemetryMetricsTest::metrics_;

TEST_F(OpenTelemetryMetricsTest, Init) {
  ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, metrics_->Name());

  // 1. init failed because opentelemetry plugin is not configured
  ASSERT_NE(0, metrics_->Init());

  // 2. init success because opentelemetry plugin is configured
  TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
  ASSERT_EQ(0, metrics_->Init());
}

trpc::ModuleMetricsInfo GetTestModuleInfo(trpc::opentelemetry::ModuleReportType type) {
  trpc::ModuleMetricsInfo info;
  info.extend_info = type;
  info.cost_time = 1000;
  info.infos["module_key"] = "module_value";
  return info;
}

TEST_F(OpenTelemetryMetricsTest, ModuleReport) {
  // 1. testing kClientStartedCount report
  auto client_started_count = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kClientStartedCount);
  ASSERT_EQ(0, metrics_->ModuleReport(client_started_count));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(client_started_count)));

  // 2. testing kClientHandledCount report
  auto client_handled_count = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kClientHandledCount);
  ASSERT_EQ(0, metrics_->ModuleReport(client_handled_count));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(client_handled_count)));

  // 3. testing kClientHandledTime report
  auto client_handled_time = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kClientHandledTime);
  ASSERT_EQ(0, metrics_->ModuleReport(client_handled_time));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(client_handled_time)));

  // 4. testing kServerStartedCount report
  auto server_started_count = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kServerStartedCount);
  ASSERT_EQ(0, metrics_->ModuleReport(server_started_count));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(server_started_count)));

  // 5. testing kServerHandledCount report
  auto server_handled_count = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kServerHandledCount);
  ASSERT_EQ(0, metrics_->ModuleReport(server_handled_count));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(server_handled_count)));

  // 6. testing kServerHandledTime report
  auto server_handled_time = GetTestModuleInfo(trpc::opentelemetry::ModuleReportType::kServerHandledTime);
  ASSERT_EQ(0, metrics_->ModuleReport(server_handled_time));
  ASSERT_EQ(0, metrics_->ModuleReport(std::move(server_handled_time)));
}

trpc::SingleAttrMetricsInfo GetTestSingleInfo(trpc::MetricsPolicy type, std::string value) {
  trpc::SingleAttrMetricsInfo info;
  info.name = "single_key";
  info.dimension = value;
  info.value = 10;
  info.policy = type;
  return info;
}

TEST_F(OpenTelemetryMetricsTest, SingleAttrReport) {
  // 1. testing report SET metrics data
  auto set_info = GetTestSingleInfo(trpc::SET, "single_set_value");
  ASSERT_EQ(0, metrics_->SingleAttrReport(set_info));
  ASSERT_EQ(0, metrics_->SingleAttrReport(std::move(set_info)));

  // 2. testing report SUM metrics data
  auto sum_info = GetTestSingleInfo(trpc::SUM, "single_sum_value");
  ASSERT_EQ(0, metrics_->SingleAttrReport(sum_info));
  ASSERT_EQ(0, metrics_->SingleAttrReport(std::move(sum_info)));

  // 3. testing report MID metrics data
  auto mid_info = GetTestSingleInfo(trpc::MID, "single_mid_value");
  ASSERT_EQ(0, metrics_->SingleAttrReport(mid_info));
  ASSERT_EQ(0, metrics_->SingleAttrReport(std::move(mid_info)));

  // 4. testing report QUANTILES metrics data
  auto quantiles_info = GetTestSingleInfo(trpc::QUANTILES, "single_quantiles_value");
  // report failed because quantiles filed is empty
  ASSERT_NE(0, metrics_->SingleAttrReport(quantiles_info));
  // report failed because the value in quantiles do not have a size of 2
  quantiles_info.quantiles = {{0.5}};
  ASSERT_NE(0, metrics_->SingleAttrReport(quantiles_info));
  // report success because the metrics data is valid
  quantiles_info.quantiles = {{0.5, 0.05}, {0.1, 0.05}};
  ASSERT_EQ(0, metrics_->SingleAttrReport(quantiles_info));
  ASSERT_EQ(0, metrics_->SingleAttrReport(std::move(quantiles_info)));

  // 5. testing report HISTOGRAM metrics data
  auto histogram_info = GetTestSingleInfo(trpc::HISTOGRAM, "single_histogram_value");
  // report failed because bucket filed is empty
  ASSERT_NE(0, metrics_->SingleAttrReport(histogram_info));
  // report success because the metrics data is valid
  histogram_info.bucket = {0.1, 0.5, 1};
  ASSERT_EQ(0, metrics_->SingleAttrReport(histogram_info));
  ASSERT_EQ(0, metrics_->SingleAttrReport(std::move(histogram_info)));

  // 6. testing report not support type
  auto max_info = GetTestSingleInfo(trpc::MAX, "single_max_value");
  ASSERT_NE(0, metrics_->SingleAttrReport(max_info));
  ASSERT_NE(0, metrics_->SingleAttrReport(std::move(max_info)));
}

trpc::MultiAttrMetricsInfo GetTestMultiInfo(trpc::MetricsPolicy type, std::string value) {
  trpc::MultiAttrMetricsInfo info;
  info.tags = {{"multi_key1", value + "1"}, {"multi_key2", value + "2"}};
  info.values = {{type, 10}};
  return info;
}

TEST_F(OpenTelemetryMetricsTest, MultiAttrReport) {
  // 1. report failed because values filed is empty
  trpc::MultiAttrMetricsInfo info;
  ASSERT_NE(0, metrics_->MultiAttrReport(info));

  // 2. testing report SET metrics data
  auto set_info = GetTestMultiInfo(trpc::SET, "multi_set_value");
  ASSERT_EQ(0, metrics_->MultiAttrReport(set_info));
  ASSERT_EQ(0, metrics_->MultiAttrReport(std::move(set_info)));

  // 3. testing report SUM metrics data
  auto sum_info = GetTestMultiInfo(trpc::SUM, "multi_sum_value");
  ASSERT_EQ(0, metrics_->MultiAttrReport(sum_info));
  ASSERT_EQ(0, metrics_->MultiAttrReport(std::move(sum_info)));

  // 4. testing report MID metrics data
  auto mid_info = GetTestMultiInfo(trpc::MID, "multi_mid_value");
  ASSERT_EQ(0, metrics_->MultiAttrReport(mid_info));
  ASSERT_EQ(0, metrics_->MultiAttrReport(std::move(mid_info)));

  // 5. testing report QUANTILES metrics data
  auto quantiles_info = GetTestMultiInfo(trpc::QUANTILES, "multi_quantiles_value");
  // report failed because quantiles filed is empty
  ASSERT_NE(0, metrics_->MultiAttrReport(quantiles_info));
  // report failed because the value in quantiles do not have a size of 2
  quantiles_info.quantiles = {{0.5}};
  ASSERT_NE(0, metrics_->MultiAttrReport(quantiles_info));
  // report success because the metrics data is valid
  quantiles_info.quantiles = {{0.5, 0.05}, {0.1, 0.05}};
  ASSERT_EQ(0, metrics_->MultiAttrReport(quantiles_info));
  ASSERT_EQ(0, metrics_->MultiAttrReport(std::move(quantiles_info)));

  // 6. testing report HISTOGRAM metrics data
  auto histogram_info = GetTestMultiInfo(trpc::HISTOGRAM, "multi_histogram_value");
  // report failed because bucket filed is empty
  ASSERT_NE(0, metrics_->MultiAttrReport(histogram_info));
  // report success because the metrics data is valid
  histogram_info.bucket = {0.1, 0.5, 1};
  ASSERT_EQ(0, metrics_->MultiAttrReport(histogram_info));
  ASSERT_EQ(0, metrics_->MultiAttrReport(std::move(histogram_info)));

  // 6. testing report not support type
  auto max_info = GetTestMultiInfo(trpc::MAX, "multi_max_value");
  ASSERT_NE(0, metrics_->MultiAttrReport(max_info));
  ASSERT_NE(0, metrics_->MultiAttrReport(std::move(max_info)));
}

}  // namespace trpc::testing
#endif
