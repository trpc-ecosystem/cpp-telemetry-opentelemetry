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

#pragma once

#include <string>

#include "opentelemetry/sdk/trace/sampler.h"

namespace trpc::opentelemetry {

constexpr char SamplerDesc[] = "TrpcSampler";

/// @brief Sampler used to determine whether a span should be sampled. It will check the following conditions in
/// sequence:
///        1. If the start attributes contain force sampled flag, it will be sampled.
///        2. If it enable to inherit the parent's sampling flag and the parent happens to be sampled, it will be
///           sampled.
///        3. If it is randomly selected for sampling, it will be sampled.
///        4. If deferred sampling is enabled, it will be marked record-only.
///        5. Do not sampled in other cases
class Sampler : public ::opentelemetry::sdk::trace::Sampler {
 public:
  struct Options {
    /// The ratio of random sampling
    double ratio = 1;
    /// Whether to allow deferred sampling
    bool enable_deferred_sample = false;
    /// Whether to not inherit the parent's sampling flag.
    bool disable_parent_sampling = false;
  };

 public:
  explicit Sampler(Options&& options);

  ::opentelemetry::sdk::trace::SamplingResult ShouldSample(
      const ::opentelemetry::trace::SpanContext& parent_context, ::opentelemetry::trace::TraceId trace_id,
      ::opentelemetry::nostd::string_view name, ::opentelemetry::trace::SpanKind span_kind,
      const ::opentelemetry::common::KeyValueIterable& attributes,
      const ::opentelemetry::trace::SpanContextKeyValueIterable& links) noexcept override;

  ::opentelemetry::nostd::string_view GetDescription() const noexcept override;

 private:
  // Calculates the sampling threshold based on the sampling ratio.
  uint64_t CalculateThreshold(double ratio);
  // Calculates a uint64 value based on the trace_id.
  uint64_t CalculateThresholdFromBuffer(const ::opentelemetry::trace::TraceId& trace_id);

  bool IsForcedSample(const ::opentelemetry::common::KeyValueIterable& attributes);

 private:
  std::string description_;
  uint64_t threshold_;
  Options options_;
};

}  // namespace trpc::opentelemetry
