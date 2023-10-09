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

#ifdef ENABLE_LOGS_PREVIEW
#include "trpc/telemetry/opentelemetry/logging/opentelemetry_logging.h"

#include "gtest/gtest.h"
#include "opentelemetry/trace/noop.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"

namespace trpc::testing {

class OpenTelemetryLoggingTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { logging_ = MakeRefCounted<OpenTelemetryLogging>(); }
  static void TearDownTestCase() {}

 protected:
  static OpenTelemetryLoggingPtr logging_;
};

OpenTelemetryLoggingPtr OpenTelemetryLoggingTest::logging_;

TEST_F(OpenTelemetryLoggingTest, Init) {
  ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, logging_->Name());

  // 1. init failed because opentelemetry plugin is not configured
  ASSERT_NE(0, logging_->Init());

  // 2. init success because opentelemetry plugin is configured
  TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
  ASSERT_EQ(0, logging_->Init());
}

TEST_F(OpenTelemetryLoggingTest, Log) {
  // 1. log without registering opentelemetry plugin
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{}});

  // registers opentelemetry plugin
  RefPtr<MockOpenTelemetryTelemetry> telemetry = MakeRefCounted<MockOpenTelemetryTelemetry>();
  OpenTelemetryTracingPtr tracing = MakeRefCounted<OpenTelemetryTracing>();
  ASSERT_EQ(0, tracing->Init());
  TelemetryFactory::GetInstance()->Register(telemetry);
  EXPECT_CALL(*telemetry, GetTracing()).WillRepeatedly(::testing::Return(tracing));

  // 2. log without extend_fields_msg
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{}});

  // 3. log with invalid span type
  std::any invalid_data = "test";
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{tracing->GetPluginID(), invalid_data}});
  ServerTracingSpan invalid_server_span;
  invalid_server_span.span = "test";
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{tracing->GetPluginID(), invalid_server_span}});

  // 4. log with span not sampled
  constexpr uint8_t buf_span[] = {1, 2, 3, 4, 5, 6, 7, 8};
  constexpr uint8_t buf_trace[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  ::opentelemetry::trace::SpanId span_id(buf_span);
  ::opentelemetry::trace::TraceId trace_id(buf_trace);
  std::unique_ptr<::opentelemetry::trace::SpanContext> not_sample_context =
      std::make_unique<::opentelemetry::trace::SpanContext>(trace_id, span_id,
                                                            ::opentelemetry::trace::TraceFlags(false), false);
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span> not_sampled_span(
      new ::opentelemetry::trace::NoopSpan(nullptr, std::move(not_sample_context)));
  ServerTracingSpan not_sampled_server_span;
  not_sampled_server_span.span = std::move(not_sampled_span);
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{tracing->GetPluginID(), not_sampled_server_span}});

  // 5. log with span not sampled but log level is error
  logging_->Log(trpc::Log::Level::error, "file", 1, "func", "msg", {{tracing->GetPluginID(), not_sampled_server_span}});

  // 6. log with span sampled
  std::unique_ptr<::opentelemetry::trace::SpanContext> sample_context =
      std::make_unique<::opentelemetry::trace::SpanContext>(trace_id, span_id, ::opentelemetry::trace::TraceFlags(true),
                                                            false);
  ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span> sampled_span_ptr(
      new ::opentelemetry::trace::NoopSpan(nullptr, std::move(sample_context)));
  ServerTracingSpan sampled_server_span;
  sampled_server_span.span = std::move(sampled_span_ptr);
  logging_->Log(trpc::Log::Level::debug, "file", 1, "func", "msg", {{tracing->GetPluginID(), sampled_server_span}});
}

}  // namespace trpc::testing
#endif
