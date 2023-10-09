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

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
#pragma once

#include "trpc/client/client_context.h"
#include "trpc/filter/filter.h"
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"

namespace trpc {

class OpenTelemetryMetricsClientFilter : public MessageClientFilter {
 public:
  int Init() override;

  std::string Name() override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  std::vector<FilterPoint> GetFilterPoint() override;

  void operator()(FilterStatus& status, FilterPoint point, const ClientContextPtr& context) override;

 private:
  void ReportClientStartedTotal(const ClientContextPtr& context, ModuleMetricsInfo& module_info);

  void ReportClientHandledSeconds(const ClientContextPtr& context, ModuleMetricsInfo& module_info);

  void ReportClientHandledTotal(const ClientContextPtr& context, ModuleMetricsInfo& module_info);

 private:
  OpenTelemetryMetricsPtr metrics_plugin_ = nullptr;

  bool enabled_ = false;
};

}  // namespace trpc

#endif
