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

#include "trpc/telemetry/opentelemetry/tracing/deferred_sample_processor.h"

#include "gtest/gtest.h"

#include "opentelemetry/exporters/ostream/span_exporter.h"
#include "opentelemetry/sdk/trace/simple_processor.h"
#include "opentelemetry/sdk/trace/span_data.h"

namespace trpc::testing {

using namespace ::opentelemetry::sdk::trace;

class MockProcessor : public SimpleSpanProcessor {
 public:
  MockProcessor() : SimpleSpanProcessor(std::make_unique<::opentelemetry::exporter::trace::OStreamSpanExporter>()) {}

  void OnEnd(std::unique_ptr<Recordable>&& span) noexcept override { reported = true; }

  static bool IsReported() { return reported; }

  static void ResetReportedFlag() { reported = false; }

 private:
  static bool reported;
};

bool MockProcessor::reported = false;

TEST(DeferredRecordableTest, Record) {
  trpc::opentelemetry::DeferredRecordable recordable(std::make_unique<SpanData>());

  // 1. tests recording sampled flag
  ASSERT_FALSE(recordable.IsSampled());
  bool test_sampled = true;
  recordable.SetIdentity(::opentelemetry::trace::SpanContext(test_sampled, false), ::opentelemetry::trace::SpanId());
  ASSERT_EQ(test_sampled, recordable.IsSampled());

  // 2. tests recording status code
  ::opentelemetry::trace::StatusCode test_code = ::opentelemetry::trace::StatusCode::kError;
  recordable.SetStatus(test_code, "");
  ASSERT_EQ(test_code, recordable.GetStatusCode());

  // 3. tests recording duration
  std::chrono::nanoseconds test_duration = std::chrono::milliseconds(100);
  recordable.SetDuration(test_duration);
  ASSERT_EQ(test_duration, recordable.GetDuration());

  // 4. tests other interface
  recordable.SetAttribute("key", "value");
  recordable.AddEvent("event", ::opentelemetry::common::SystemTimestamp(),
                      ::opentelemetry::common::NoopKeyValueIterable());
  recordable.AddLink(::opentelemetry::trace::SpanContext(false, false),
                     ::opentelemetry::common::NoopKeyValueIterable());
  recordable.SetName("name");
  recordable.SetSpanKind(::opentelemetry::trace::SpanKind::kClient);
  recordable.SetResource(
      ::opentelemetry::sdk::resource::Resource::Create(::opentelemetry::sdk::common::AttributeMap()));
  recordable.SetStartTime(::opentelemetry::common::SystemTimestamp());
  auto scope = InstrumentationScope::Create("", "");
  recordable.SetInstrumentationScope(*scope);
}

TEST(DeferredSampleProcessorTest, Report) {
  bool enable_sample_error = true;
  std::chrono::microseconds sample_slow_duration = std::chrono::microseconds(100);
  trpc::opentelemetry::DeferredSampleProcessor::Options options = {.enable_sample_error = enable_sample_error,
                                                                   .sample_slow_duration = sample_slow_duration};
  trpc::opentelemetry::DeferredSampleProcessor processor(std::make_unique<MockProcessor>(), std::move(options));

  // already sampled
  MockProcessor::ResetReportedFlag();
  auto sampled_recordable = processor.MakeRecordable();
  ASSERT_NE(nullptr, sampled_recordable);
  sampled_recordable->SetIdentity(::opentelemetry::trace::SpanContext(true, false), ::opentelemetry::trace::SpanId());
  processor.OnEnd(std::move(sampled_recordable));
  ASSERT_TRUE(MockProcessor::IsReported());

  // not sampled previously but error occurs
  MockProcessor::ResetReportedFlag();
  auto error_recordable = processor.MakeRecordable();
  ASSERT_NE(nullptr, error_recordable);
  error_recordable->SetIdentity(::opentelemetry::trace::SpanContext(false, false), ::opentelemetry::trace::SpanId());
  error_recordable->SetStatus(::opentelemetry::trace::StatusCode::kError, "");
  processor.OnEnd(std::move(error_recordable));
  ASSERT_TRUE(MockProcessor::IsReported());

  // not sampled previously and no errors occurred, but a high latency is observed
  MockProcessor::ResetReportedFlag();
  auto duration_recordable = processor.MakeRecordable();
  ASSERT_NE(nullptr, duration_recordable);
  duration_recordable->SetIdentity(::opentelemetry::trace::SpanContext(false, false), ::opentelemetry::trace::SpanId());
  duration_recordable->SetStatus(::opentelemetry::trace::StatusCode::kOk, "");
  duration_recordable->SetDuration(std::chrono::microseconds(200));
  processor.OnEnd(std::move(duration_recordable));
  ASSERT_TRUE(MockProcessor::IsReported());

  // not sampled previously, and no errors or high latency were observed
  MockProcessor::ResetReportedFlag();
  auto com_recordable = processor.MakeRecordable();
  ASSERT_NE(nullptr, com_recordable);
  com_recordable->SetIdentity(::opentelemetry::trace::SpanContext(false, false), ::opentelemetry::trace::SpanId());
  com_recordable->SetStatus(::opentelemetry::trace::StatusCode::kOk, "");
  com_recordable->SetDuration(std::chrono::microseconds(50));
  processor.OnEnd(std::move(com_recordable));
  ASSERT_FALSE(MockProcessor::IsReported());

  // tests other interface
  auto other_recordable = processor.MakeRecordable();
  processor.OnStart(*other_recordable, ::opentelemetry::trace::SpanContext(false, false));
  processor.ForceFlush(std::chrono::microseconds(50));
  processor.Shutdown(std::chrono::microseconds(50));
}

}  // namespace trpc::testing
