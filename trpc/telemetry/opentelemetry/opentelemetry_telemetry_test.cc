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

#include "gtest/gtest.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/util/log/logging.h"

namespace trpc::testing {

class OpenTelemetryTelemetryTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    telemetry_ = MakeRefCounted<OpenTelemetryTelemetry>();
    ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, telemetry_->Name());
  }

  static void TearDownTestCase() {}

 protected:
  static OpenTelemetryTelemetryPtr telemetry_;
};
OpenTelemetryTelemetryPtr OpenTelemetryTelemetryTest::telemetry_;

TEST_F(OpenTelemetryTelemetryTest, Init) {
  // 1. init failed because opentelemetry plugin is not configured
  ASSERT_NE(0, telemetry_->Init());

  // 2. init success because opentelemetry plugin had configured
  TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
  ASSERT_TRUE(log::Init());
  ASSERT_EQ(0, telemetry_->Init());

  telemetry_->Start();
  telemetry_->Stop();
  telemetry_->Destroy();

  log::Destroy();
}

TEST_F(OpenTelemetryTelemetryTest, Get) {
  ASSERT_NE(telemetry_->GetTracing(), nullptr);

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  ASSERT_NE(telemetry_->GetMetrics(), nullptr);
#else
  ASSERT_EQ(telemetry_->GetMetrics(), nullptr);
#endif

#ifdef ENABLE_LOGS_PREVIEW
  ASSERT_NE(telemetry_->GetLog(), nullptr);
#else
  ASSERT_EQ(telemetry_->GetLog(), nullptr);
#endif
}

}  // namespace trpc::testing
