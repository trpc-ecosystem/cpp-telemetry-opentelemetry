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
#include "trpc/telemetry/opentelemetry/metrics/client_filter.h"

#include "gtest/gtest.h"
#include "trpc/codec/trpc/trpc_client_codec.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"
#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"

namespace trpc::testing {

class OpenTelemetryMetricsClientFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);

    // registers a telemetry to ensure that OpenTelemetryMetricsClientFilter initializes success
    auto telemetry = MakeRefCounted<MockOpenTelemetryTelemetry>();
    metrics_ = MakeRefCounted<OpenTelemetryMetrics>();
    ASSERT_EQ(0, metrics_->Init());
    TelemetryFactory::GetInstance()->Register(telemetry);
    EXPECT_CALL(*telemetry, GetMetrics()).WillOnce(::testing::Return(metrics_));
    client_filter_ = std::make_shared<OpenTelemetryMetricsClientFilter>();
    ASSERT_EQ(0, client_filter_->Init());
  }
  static void TearDownTestCase() {}

 protected:
  static MessageClientFilterPtr client_filter_;
  static MetricsPtr metrics_;
};

MessageClientFilterPtr OpenTelemetryMetricsClientFilterTest::client_filter_;
MetricsPtr OpenTelemetryMetricsClientFilterTest::metrics_;

TEST_F(OpenTelemetryMetricsClientFilterTest, Init) {
  ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, client_filter_->Name());
  std::vector<FilterPoint> points = client_filter_->GetFilterPoint();
  ASSERT_EQ(2, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_POST_RPC_INVOKE) != points.end());
}

TEST_F(OpenTelemetryMetricsClientFilterTest, Report) {
  auto trpc_codec = std::make_shared<trpc::TrpcClientCodec>();
  trpc::ClientContextPtr context = trpc::MakeRefCounted<trpc::ClientContext>(trpc_codec);
  context->SetCallerName("trpc.test.helloworld.client");
  context->SetCalleeName("trpc.test.helloworld.Greeter");
  context->SetFuncName("SayHello");

  FilterStatus status;
  client_filter_->operator()(status, FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);

  trpc::Status frame_status;
  frame_status.SetFrameworkRetCode(101);
  context->SetStatus(frame_status);
  client_filter_->operator()(status, FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
}

}  // namespace trpc::testing
#endif
