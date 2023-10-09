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
#include "trpc/telemetry/opentelemetry/logging/grpc_log_exporter.h"

#include "opentelemetry/exporters/otlp/otlp_log_recordable.h"
#include "opentelemetry/exporters/otlp/otlp_recordable_utils.h"
#include "trpc/client/make_client_context.h"
#include "trpc/client/trpc_client.h"
#include "trpc/util/log/logging.h"

#include "trpc/telemetry/opentelemetry/logging/common.h"

namespace trpc::opentelemetry {

GrpcLogExporter::GrpcLogExporter(const ServiceProxyOption& options) {
  logs_service_proxy_ = GetTrpcClient()->GetProxy<::opentelemetry::proto::collector::logs::v1::LogsServiceServiceProxy>(
      options.name, &options);
  TRPC_ASSERT(logs_service_proxy_);
}

GrpcLogExporter::GrpcLogExporter(
    std::shared_ptr<::opentelemetry::proto::collector::logs::v1::LogsServiceServiceProxy> proxy)
    : logs_service_proxy_(proxy) {
  TRPC_ASSERT(logs_service_proxy_);
}

std::unique_ptr<::opentelemetry::sdk::logs::Recordable> GrpcLogExporter::MakeRecordable() noexcept {
  return std::make_unique<::opentelemetry::exporter::otlp::OtlpLogRecordable>();
}

::opentelemetry::sdk::common::ExportResult GrpcLogExporter::Export(
    const ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::logs::Recordable>>& records) noexcept {
  if (isShutdown()) {
    TRPC_LOG_ERROR("[OpenTelemetry gRPC log record] Exporting " << records.size()
                                                                << " record(s) failed, exporter is shutdown");
    return ::opentelemetry::sdk::common::ExportResult::kFailure;
  }
  if (records.empty()) {
    return ::opentelemetry::sdk::common::ExportResult::kSuccess;
  }

  ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceRequest request;
  ::opentelemetry::exporter::otlp::OtlpRecordableUtils::PopulateRequest(records, &request);

  ClientContextPtr client_context = MakeClientContext(logs_service_proxy_);

  ::opentelemetry::proto::collector::logs::v1::ExportLogsServiceResponse response;

  ::trpc::Status status = logs_service_proxy_->Export(client_context, request, &response);

  if (!status.OK()) {
    TRPC_LOG_ERROR("[OpenTelemetry LOGS GRPC Exporter] Export() failed: " << status.ToString());
    return ::opentelemetry::sdk::common::ExportResult::kFailure;
  }
  return ::opentelemetry::sdk::common::ExportResult::kSuccess;
}

bool GrpcLogExporter::ForceFlush(std::chrono::microseconds timeout) noexcept { return true; }

bool GrpcLogExporter::Shutdown(std::chrono::microseconds timeout) noexcept {
  const std::lock_guard<::opentelemetry::common::SpinLockMutex> locked(lock_);
  is_shutdown_ = true;
  return true;
}

bool GrpcLogExporter::isShutdown() const noexcept {
  const std::lock_guard<::opentelemetry::common::SpinLockMutex> locked(lock_);
  return is_shutdown_;
}

}  // namespace trpc::opentelemetry
#endif
