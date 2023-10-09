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

#include "trpc/telemetry/opentelemetry/tracing/grpc_trace_exporter.h"

#include "opentelemetry/exporters/otlp/otlp_recordable.h"
#include "opentelemetry/exporters/otlp/otlp_recordable_utils.h"
#include "trpc/client/make_client_context.h"
#include "trpc/client/trpc_client.h"
#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/util/log/logging.h"

namespace trpc::opentelemetry {

GrpcTraceExporter::GrpcTraceExporter(const ServiceProxyOption& options) {
  trace_service_proxy_ =
      GetTrpcClient()->GetProxy<::opentelemetry::proto::collector::trace::v1::TraceServiceServiceProxy>(options.name,
                                                                                                        &options);
  TRPC_ASSERT(trace_service_proxy_);
}

GrpcTraceExporter::GrpcTraceExporter(
    std::shared_ptr<::opentelemetry::proto::collector::trace::v1::TraceServiceServiceProxy> proxy)
    : trace_service_proxy_(proxy) {
  TRPC_ASSERT(trace_service_proxy_);
}

std::unique_ptr<::opentelemetry::sdk::trace::Recordable> GrpcTraceExporter::MakeRecordable() noexcept {
  return std::make_unique<::opentelemetry::exporter::otlp::OtlpRecordable>();
}

::opentelemetry::sdk::common::ExportResult GrpcTraceExporter::Export(
    const ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>>& spans) noexcept {
  if (isShutdown()) {
    TRPC_LOG_ERROR("[OpenTelemetry gRPC] Exporting " << spans.size() << " span(s) failed, exporter is shutdown");
    return ::opentelemetry::sdk::common::ExportResult::kFailure;
  }

  if (spans.empty()) {
    return ::opentelemetry::sdk::common::ExportResult::kSuccess;
  }

  ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceRequest request;
  ::opentelemetry::exporter::otlp::OtlpRecordableUtils::PopulateRequest(spans, &request);

  ClientContextPtr client_context = trpc::MakeClientContext(trace_service_proxy_);

  ::opentelemetry::proto::collector::trace::v1::ExportTraceServiceResponse response;

  ::trpc::Status status = trace_service_proxy_->Export(client_context, request, &response);

  if (!status.OK()) {
    TRPC_LOG_ERROR("[OpenTelemetry TRACE GRPC Exporter] Export() failed: " << status.ToString());
    return ::opentelemetry::sdk::common::ExportResult::kFailure;
  }
  return ::opentelemetry::sdk::common::ExportResult::kSuccess;
}

bool GrpcTraceExporter::ForceFlush(std::chrono::microseconds timeout) noexcept { return true; }

bool GrpcTraceExporter::Shutdown(std::chrono::microseconds timeout) noexcept {
  const std::lock_guard<::opentelemetry::common::SpinLockMutex> locked(lock_);
  is_shutdown_ = true;
  return true;
}

bool GrpcTraceExporter::isShutdown() const noexcept {
  const std::lock_guard<::opentelemetry::common::SpinLockMutex> locked(lock_);
  return is_shutdown_;
}

}  // namespace trpc::opentelemetry
