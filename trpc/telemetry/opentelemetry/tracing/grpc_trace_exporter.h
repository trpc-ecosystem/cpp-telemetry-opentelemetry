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

#include "opentelemetry/common/spin_lock_mutex.h"
#include "opentelemetry/sdk/trace/exporter.h"
#include "trpc/client/service_proxy_option.h"
#include "trpc/telemetry/opentelemetry/tracing/trace_service.trpc.pb.h"

namespace trpc::opentelemetry {

/// @brief Trace exporter based on the trpc framework that uses the gRPC protocol for reporting.
class GrpcTraceExporter final : public ::opentelemetry::sdk::trace::SpanExporter {
 public:
  explicit GrpcTraceExporter(const ServiceProxyOption& options);

  explicit GrpcTraceExporter(
      std::shared_ptr<::opentelemetry::proto::collector::trace::v1::TraceServiceServiceProxy> proxy);

  std::unique_ptr<::opentelemetry::sdk::trace::Recordable> MakeRecordable() noexcept override;

  ::opentelemetry::sdk::common::ExportResult Export(
      const ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>>&
          spans) noexcept override;

  bool ForceFlush(std::chrono::microseconds timeout = std::chrono::microseconds::max()) noexcept override;

  bool Shutdown(std::chrono::microseconds timeout = std::chrono::microseconds::max()) noexcept override;

 private:
  // Checks if exporter had shutdown
  bool isShutdown() const noexcept;

 private:
  std::shared_ptr<::opentelemetry::proto::collector::trace::v1::TraceServiceServiceProxy> trace_service_proxy_;

  bool is_shutdown_ = false;
  mutable ::opentelemetry::common::SpinLockMutex lock_;
};

}  // namespace trpc::opentelemetry
