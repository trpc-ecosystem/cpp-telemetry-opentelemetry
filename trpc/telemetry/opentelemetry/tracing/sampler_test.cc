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

#include "trpc/telemetry/opentelemetry/tracing/sampler.h"

#include <map>

#include "gtest/gtest.h"

#include "opentelemetry/trace/span_context_kv_iterable.h"
#include "trpc/telemetry/opentelemetry/tracing/common.h"

namespace trpc::testing {

trpc::opentelemetry::Sampler::Options GetOptions(double ratio, bool disable_parent_sampling,
                                                 bool enable_deferred_sample) {
  trpc::opentelemetry::Sampler::Options options;
  options.ratio = ratio;
  options.disable_parent_sampling = disable_parent_sampling;
  options.enable_deferred_sample = enable_deferred_sample;
  return options;
}

::opentelemetry::sdk::trace::SamplingResult TestShouldSample(trpc::opentelemetry::Sampler::Options&& options,
                                                             const ::opentelemetry::trace::SpanContext& context,
                                                             const std::map<std::string, std::string>& attrubutes) {
  trpc::opentelemetry::Sampler sampler(std::move(options));
  ::opentelemetry::trace::NullSpanContext links;
  auto result = sampler.ShouldSample(
      context, context.trace_id(), "test", ::opentelemetry::trace::SpanKind::kServer,
      ::opentelemetry::common::KeyValueIterableView<std::map<std::string, std::string>>(attrubutes), links);
  return result;
}

TEST(SampleTest, ForceSample) {
  // the attributes have force sampled flag
  ::opentelemetry::trace::SpanContext context(false, false);
  auto result =
      TestShouldSample(GetOptions(0, false, false), context, {{trpc::opentelemetry::kForceSampleKey, "force"}});
  ASSERT_EQ(::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, result.decision);
  ASSERT_EQ(nullptr, result.attributes);
}

TEST(SampleTest, ParentSample) {
  // the attributes do not have force sampled flag, but it enable to inherit the parent's sampling flag and the parent
  // happens to be sampled
  ::opentelemetry::trace::SpanContext context(true, false);
  auto result = TestShouldSample(GetOptions(0, false, false), context, {});
  ASSERT_EQ(::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, result.decision);
  ASSERT_EQ(nullptr, result.attributes);
}

TEST(SampleTest, RamdomSample) {
  // the attributes do not have force sampled flag and do not enable to inherit the parent's sampling flag
  // but it is randomly selected for sampling
  ::opentelemetry::trace::SpanContext context(false, false);
  auto result = TestShouldSample(GetOptions(1, true, false), context, {});
  ASSERT_EQ(::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, result.decision);
  ASSERT_EQ(nullptr, result.attributes);
}

TEST(SampleTest, DeferredSample) {
  // does not meet the conditions for direct sampling, but deferred sampling is enabled
  ::opentelemetry::trace::SpanContext context(false, false);
  auto result = TestShouldSample(GetOptions(0, true, true), context, {});
  ASSERT_EQ(::opentelemetry::sdk::trace::Decision::RECORD_ONLY, result.decision);
  ASSERT_EQ(nullptr, result.attributes);
}

TEST(SampleTest, NotSample) {
  // does not meet the conditions for direct sampling and deferred sampling is not enabled
  ::opentelemetry::trace::SpanContext context(false, false);
  auto result = TestShouldSample(GetOptions(0, true, false), context, {});
  ASSERT_EQ(::opentelemetry::sdk::trace::Decision::DROP, result.decision);
  ASSERT_EQ(nullptr, result.attributes);
}

}  // namespace trpc::testing
