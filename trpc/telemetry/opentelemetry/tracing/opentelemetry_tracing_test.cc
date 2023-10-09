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

#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"

#include "gtest/gtest.h"
#include "trpc/common/config/trpc_config.h"

namespace trpc::testing {

class OpenTelemetryTracingTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { tracing_ = MakeRefCounted<OpenTelemetryTracing>(); }
  static void TearDownTestCase() {}

 protected:
  static OpenTelemetryTracingPtr tracing_;
};
OpenTelemetryTracingPtr OpenTelemetryTracingTest::tracing_;

TEST_F(OpenTelemetryTracingTest, Init) {
  // 1. init failed because opentelemetry plugin is not configured
  ASSERT_NE(0, tracing_->Init());

  // 2. init success because opentelemetry plugin had configured
  TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
  ASSERT_EQ(0, tracing_->Init());
}

TEST_F(OpenTelemetryTracingTest, MakeTracer) {
  // test for using empty service_name
  std::string err_msg;
  auto empty_tracer = tracing_->MakeTracer("", err_msg);
  ASSERT_TRUE(empty_tracer);

  // test for getting tracer normally
  std::string service_a = "service_a";
  auto ser_a_tracer_1 = tracing_->MakeTracer(service_a.c_str(), err_msg);
  ASSERT_TRUE(ser_a_tracer_1);
  auto ser_a_tracer_2 = tracing_->MakeTracer(service_a.c_str(), err_msg);
  ASSERT_TRUE(ser_a_tracer_2);
  // identical service_names will use the same tracer
  ASSERT_EQ(ser_a_tracer_1, ser_a_tracer_2);

  std::string service_b = "service_b";
  auto ser_b_tracer = tracing_->MakeTracer(service_b.c_str(), err_msg);
  ASSERT_TRUE(ser_b_tracer);
  // different service_names will use different tracers
  ASSERT_NE(ser_a_tracer_1, ser_b_tracer);
}

}  // namespace trpc::testing
