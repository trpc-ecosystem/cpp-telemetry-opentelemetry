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

#include "trpc/telemetry/opentelemetry/opentelemetry_client_filter.h"

#include "gtest/gtest.h"
#include "trpc/codec/trpc/trpc_client_codec.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry.h"

namespace trpc::testing {

class OpenTelemetryClientFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);
    ASSERT_TRUE(log::Init());

    // registers OpenTelemetryTelemetry
    TelemetryPtr telemetry = MakeRefCounted<OpenTelemetryTelemetry>();
    TelemetryFactory::GetInstance()->Register(telemetry);
    ASSERT_TRUE(telemetry->Init() == 0);

    // initializes filter
    filter_ = std::make_shared<OpenTelemetryClientFilter>();
    ASSERT_TRUE(filter_->Init() == 0);
  }
  static void TearDownTestCase() { log::Destroy(); }

 protected:
  static std::shared_ptr<OpenTelemetryClientFilter> filter_;
};

std::shared_ptr<OpenTelemetryClientFilter> OpenTelemetryClientFilterTest::filter_;

TEST_F(OpenTelemetryClientFilterTest, GetFilterPoint) {
  auto points = filter_->GetFilterPoint();
  ASSERT_EQ(2, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_POST_RPC_INVOKE) != points.end());
}

TEST_F(OpenTelemetryClientFilterTest, TestDoFilter) {
  auto trpc_codec = std::make_shared<trpc::TrpcClientCodec>();
  ClientContextPtr context = MakeRefCounted<ClientContext>(trpc_codec);

  FilterStatus status;
  (*filter_)(status, FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  (*filter_)(status, FilterPoint::CLIENT_POST_RPC_INVOKE, context);
}

}  // namespace trpc::testing
