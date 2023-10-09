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

#include <chrono>

#include "opentelemetry/sdk/trace/processor.h"
#include "opentelemetry/sdk/trace/span_data.h"

namespace trpc::opentelemetry {

/// @brief Implementation of the deferred sample recordable. It records additional information such as whether the span
///        was sampled, error codes, and duration.
class DeferredRecordable : public ::opentelemetry::sdk::trace::Recordable {
 public:
  /// @brief The constructor of DeferredRecordable
  /// @param recordable the object responsible for executing the actual recordable logic internally
  explicit DeferredRecordable(std::unique_ptr<::opentelemetry::sdk::trace::Recordable>&& recordable);

  /// @brief Checks if the span had sampled.
  bool IsSampled();

  /// @brief Gets the status code.
  ::opentelemetry::trace::StatusCode GetStatusCode();

  /// @brief Gets the duration.
  std::chrono::nanoseconds GetDuration();

  /// @brief Gets the inner recordable.
  std::unique_ptr<::opentelemetry::sdk::trace::Recordable> GetRecordable();

  void SetIdentity(const ::opentelemetry::trace::SpanContext& span_context,
                   ::opentelemetry::trace::SpanId parent_span_id) noexcept override;

  void SetAttribute(::opentelemetry::nostd::string_view key,
                    const ::opentelemetry::common::AttributeValue& value) noexcept override;

  void AddEvent(::opentelemetry::nostd::string_view name, ::opentelemetry::common::SystemTimestamp timestamp,
                const ::opentelemetry::common::KeyValueIterable& attributes) noexcept override;

  void AddLink(const ::opentelemetry::trace::SpanContext& span_context,
               const ::opentelemetry::common::KeyValueIterable& attributes) noexcept override;

  void SetStatus(::opentelemetry::trace::StatusCode code,
                 ::opentelemetry::nostd::string_view description) noexcept override;

  void SetName(::opentelemetry::nostd::string_view name) noexcept override;

  void SetSpanKind(::opentelemetry::trace::SpanKind span_kind) noexcept override;

  void SetResource(const ::opentelemetry::sdk::resource::Resource& resource) noexcept override;

  void SetStartTime(::opentelemetry::common::SystemTimestamp start_time) noexcept override;

  void SetDuration(std::chrono::nanoseconds duration) noexcept override;

  void SetInstrumentationScope(
      const ::opentelemetry::sdk::trace::InstrumentationScope& instrumentation_scope) noexcept override;

 private:
  std::unique_ptr<Recordable> inner_recordable_;

  bool sampled_ = false;
  ::opentelemetry::trace::StatusCode code_;
  std::chrono::nanoseconds duration_;
};

/// @brief Implementation of the deferred sample processor, which allows further making sample decisions based on
///        deferred sample Options, such as errors and high latency.
class DeferredSampleProcessor : public ::opentelemetry::sdk::trace::SpanProcessor {
 public:
  /// Options for DeferredSample
  struct Options {
    /// Whether to sample spans that encounter errors
    bool enable_sample_error = false;
    /// The sampled threshold for high latency spans.
    std::chrono::microseconds sample_slow_duration = (std::chrono::microseconds::max)();
  };

 public:
  /// @brief The constructor of DeferredSampleProcessor
  /// @param processor the object responsible for executing the actual processor logic internally
  /// @param sample_options options for deferred sample
  DeferredSampleProcessor(std::unique_ptr<::opentelemetry::sdk::trace::SpanProcessor>&& processor,
                          Options&& sample_options);

  std::unique_ptr<::opentelemetry::sdk::trace::Recordable> MakeRecordable() noexcept override;

  void OnStart(::opentelemetry::sdk::trace::Recordable& span,
               const ::opentelemetry::trace::SpanContext& parent_context) noexcept override;

  void OnEnd(std::unique_ptr<::opentelemetry::sdk::trace::Recordable>&& span) noexcept override;

  bool ForceFlush(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override;

  bool Shutdown(std::chrono::microseconds timeout = (std::chrono::microseconds::max)()) noexcept override;

 private:
  bool ShouldDeferredSampler(DeferredRecordable* recordable) noexcept;

 private:
  std::unique_ptr<::opentelemetry::sdk::trace::SpanProcessor> inner_processor_;
  Options sample_options_;
};

}  // namespace trpc::opentelemetry
