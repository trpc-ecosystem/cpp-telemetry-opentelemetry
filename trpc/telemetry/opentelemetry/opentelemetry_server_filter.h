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

#include "trpc/filter/filter.h"

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
#include "trpc/telemetry/opentelemetry/metrics/server_filter.h"
#endif
#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/tracing/server_filter.h"

namespace trpc {

class OpenTelemetryServerFilter : public MessageServerFilter {
 public:
  OpenTelemetryServerFilter();

  int Init() override;

  std::string Name() override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  std::vector<FilterPoint> GetFilterPoint() override;

  void operator()(FilterStatus& status, FilterPoint point, const ServerContextPtr& context) override;

 private:
  std::shared_ptr<OpenTelemetryTracingServerFilter> tracing_filter_;
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  std::shared_ptr<OpenTelemetryMetricsServerFilter> metrics_filter_;
#endif
};

}  // namespace trpc
