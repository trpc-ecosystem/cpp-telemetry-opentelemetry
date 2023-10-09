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

#include <cmath>

#include "trpc/telemetry/opentelemetry/tracing/common.h"

namespace trpc::opentelemetry {

Sampler::Sampler(Options&& options) : threshold_(CalculateThreshold(options.ratio)), options_(std::move(options)) {
  description_ = SamplerDesc;
}

uint64_t Sampler::CalculateThreshold(double ratio) {
  if (ratio <= 0.0) return 0;
  if (ratio >= 1.0) return UINT64_MAX;

  const double product = UINT32_MAX * ratio;
  double hi_bits, lo_bits = ldexp(modf(product, &hi_bits), 32) + product;
  return (static_cast<uint64_t>(hi_bits) << 32) + static_cast<uint64_t>(lo_bits);
}

uint64_t Sampler::CalculateThresholdFromBuffer(const ::opentelemetry::trace::TraceId& trace_id) {
  static_assert(::opentelemetry::trace::TraceId::kSize >= 8, "TraceID must be at least 8 bytes long.");

  uint64_t res = 0;
  std::memcpy(&res, &trace_id, 8);
  double ratio = static_cast<double>(res) / UINT64_MAX;
  return CalculateThreshold(ratio);
}

::opentelemetry::sdk::trace::SamplingResult Sampler::ShouldSample(
    const ::opentelemetry::trace::SpanContext& parent_context, ::opentelemetry::trace::TraceId trace_id,
    ::opentelemetry::nostd::string_view name, ::opentelemetry::trace::SpanKind span_kind,
    const ::opentelemetry::common::KeyValueIterable& attributes,
    const ::opentelemetry::trace::SpanContextKeyValueIterable& links) noexcept {
  // if the user forces the request to be sampled, then it must be sampled.
  if (IsForcedSample(attributes)) {
    return {::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, nullptr};
  }

  // if the parent has been sampled, then this span should also be sampled.
  if (!options_.disable_parent_sampling && parent_context.IsSampled()) {
    return {::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, nullptr};
  }

  // ramdom sampling
  if (threshold_ != 0 && CalculateThresholdFromBuffer(trace_id) <= threshold_) {
    return {::opentelemetry::sdk::trace::Decision::RECORD_AND_SAMPLE, nullptr};
  }

  // if deferred sampling is enabled, trace information should be recorded and further sampling decision should be made
  // before reporting.
  if (options_.enable_deferred_sample) {
    return {::opentelemetry::sdk::trace::Decision::RECORD_ONLY, nullptr};
  }

  // do not sample in other cases
  return {::opentelemetry::sdk::trace::Decision::DROP, nullptr};
}

::opentelemetry::nostd::string_view Sampler::GetDescription() const noexcept { return description_; }

bool Sampler::IsForcedSample(const ::opentelemetry::common::KeyValueIterable& attributes) {
  bool dyeing_flag = false;
  attributes.ForEachKeyValue([this, &dyeing_flag](::opentelemetry::nostd::string_view key,
                                                  ::opentelemetry::common::AttributeValue value) noexcept {
    if (trpc::opentelemetry::kForceSampleKey == key) {
      dyeing_flag = true;
      return false;
    }
    return true;
  });
  return dyeing_flag;
}

}  // namespace trpc::opentelemetry
