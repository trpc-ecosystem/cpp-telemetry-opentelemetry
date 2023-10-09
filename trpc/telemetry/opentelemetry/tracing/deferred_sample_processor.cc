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

namespace trpc::opentelemetry {

using namespace ::opentelemetry::sdk::trace;

DeferredRecordable::DeferredRecordable(std::unique_ptr<Recordable>&& recordable)
    : inner_recordable_(std::move(recordable)) {}

bool DeferredRecordable::IsSampled() { return sampled_; }

::opentelemetry::trace::StatusCode DeferredRecordable::GetStatusCode() { return code_; }

std::chrono::nanoseconds DeferredRecordable::GetDuration() { return duration_; }

std::unique_ptr<Recordable> DeferredRecordable::GetRecordable() { return std::move(inner_recordable_); }

void DeferredRecordable::SetIdentity(const ::opentelemetry::trace::SpanContext& span_context,
                                     ::opentelemetry::trace::SpanId parent_span_id) noexcept {
  sampled_ = span_context.IsSampled();
  inner_recordable_->SetIdentity(span_context, parent_span_id);
}

void DeferredRecordable::SetAttribute(::opentelemetry::nostd::string_view key,
                                      const ::opentelemetry::common::AttributeValue& value) noexcept {
  inner_recordable_->SetAttribute(key, value);
}

void DeferredRecordable::AddEvent(::opentelemetry::nostd::string_view name,
                                  ::opentelemetry::common::SystemTimestamp timestamp,
                                  const ::opentelemetry::common::KeyValueIterable& attributes) noexcept {
  inner_recordable_->AddEvent(name, timestamp, attributes);
}

void DeferredRecordable::AddLink(const ::opentelemetry::trace::SpanContext& span_context,
                                 const ::opentelemetry::common::KeyValueIterable& attributes) noexcept {
  inner_recordable_->AddLink(span_context, attributes);
}

void DeferredRecordable::SetStatus(::opentelemetry::trace::StatusCode code,
                                   ::opentelemetry::nostd::string_view description) noexcept {
  code_ = code;
  inner_recordable_->SetStatus(code, description);
}

void DeferredRecordable::SetName(::opentelemetry::nostd::string_view name) noexcept {
  inner_recordable_->SetName(name);
}

void DeferredRecordable::SetSpanKind(::opentelemetry::trace::SpanKind span_kind) noexcept {
  inner_recordable_->SetSpanKind(span_kind);
}

void DeferredRecordable::SetResource(const ::opentelemetry::sdk::resource::Resource& resource) noexcept {
  inner_recordable_->SetResource(resource);
}

void DeferredRecordable::SetStartTime(::opentelemetry::common::SystemTimestamp start_time) noexcept {
  inner_recordable_->SetStartTime(start_time);
}

void DeferredRecordable::SetDuration(std::chrono::nanoseconds duration) noexcept {
  duration_ = duration;
  inner_recordable_->SetDuration(duration);
}

void DeferredRecordable::SetInstrumentationScope(const InstrumentationScope& instrumentation_scope) noexcept {
  inner_recordable_->SetInstrumentationScope(instrumentation_scope);
}

DeferredSampleProcessor::DeferredSampleProcessor(std::unique_ptr<SpanProcessor>&& processor, Options&& sample_options)
    : inner_processor_(std::move(processor)), sample_options_(std::move(sample_options)) {}

std::unique_ptr<Recordable> DeferredSampleProcessor::MakeRecordable() noexcept {
  return std::make_unique<DeferredRecordable>(std::move(inner_processor_->MakeRecordable()));
}

void DeferredSampleProcessor::OnStart(Recordable& span,
                                      const ::opentelemetry::trace::SpanContext& parent_context) noexcept {
  return inner_processor_->OnStart(span, parent_context);
}

void DeferredSampleProcessor::OnEnd(std::unique_ptr<Recordable>&& span) noexcept {
  auto recordable = dynamic_cast<DeferredRecordable*>(span.get());
  if (recordable == nullptr) {
    return;
  }
  if (ShouldDeferredSampler(recordable)) {
    inner_processor_->OnEnd(std::move(recordable->GetRecordable()));
  }
  return;
}

bool DeferredSampleProcessor::ShouldDeferredSampler(DeferredRecordable* recordable) noexcept {
  // already sampled
  if (recordable->IsSampled()) {
    return true;
  }

  // error occurred
  if (sample_options_.enable_sample_error &&
      recordable->GetStatusCode() != ::opentelemetry::v1::trace::StatusCode::kOk) {
    return true;
  }

  // time spent above the threshold
  if (recordable->GetDuration() > sample_options_.sample_slow_duration) {
    return true;
  }

  return false;
}

bool DeferredSampleProcessor::ForceFlush(std::chrono::microseconds timeout) noexcept {
  return inner_processor_->ForceFlush(timeout);
}

bool DeferredSampleProcessor::Shutdown(std::chrono::microseconds timeout) noexcept {
  return inner_processor_->Shutdown(timeout);
}

}  // namespace trpc::opentelemetry
