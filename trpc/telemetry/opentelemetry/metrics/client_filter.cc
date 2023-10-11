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
#include "trpc/telemetry/opentelemetry/metrics/client_filter.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/util/time.h"

#include "trpc/telemetry/opentelemetry/metrics/common.h"

namespace trpc {

int OpenTelemetryMetricsClientFilter::Init() {
  auto telemetry = TelemetryFactory::GetInstance()->Get(trpc::opentelemetry::kOpenTelemetryTelemetryName);
  if (!telemetry) {
    TRPC_FMT_ERROR("plugin {} is not registered", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }

  metrics_plugin_ = trpc::dynamic_pointer_cast<OpenTelemetryMetrics>(telemetry->GetMetrics());
  auto& config = metrics_plugin_->GetConfig();
  enabled_ = config.metrics_config.enabled;

  return 0;
}

std::vector<FilterPoint> OpenTelemetryMetricsClientFilter::GetFilterPoint() {
  std::vector<FilterPoint> points = {FilterPoint::CLIENT_PRE_RPC_INVOKE, FilterPoint::CLIENT_POST_RPC_INVOKE};
  return points;
}

void OpenTelemetryMetricsClientFilter::operator()(FilterStatus& status, FilterPoint point,
                                                  const ClientContextPtr& context) {
  status = FilterStatus::CONTINUE;

  if (!enabled_) {
    return;
  }

  ModuleMetricsInfo module_info;
  module_info.source = kMetricsCallerSource;
  module_info.infos[trpc::opentelemetry::kCallerService] = context->GetCallerName();
  module_info.infos[trpc::opentelemetry::kCallerMethod] = context->GetCallerFuncName();
  module_info.infos[trpc::opentelemetry::kCalleeService] = context->GetCalleeName();
  module_info.infos[trpc::opentelemetry::kCalleeMethod] = context->GetFuncName();

  if (point == FilterPoint::CLIENT_PRE_RPC_INVOKE) {
    ReportClientStartedTotal(context, module_info);
  } else if (point == FilterPoint::CLIENT_POST_RPC_INVOKE) {
    ReportClientHandledSeconds(context, module_info);
    ReportClientHandledTotal(context, module_info);
  }

  status = FilterStatus::CONTINUE;
}

void OpenTelemetryMetricsClientFilter::ReportClientStartedTotal(const ClientContextPtr& context,
                                                                ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kClientStartedCount;
  metrics_plugin_->ModuleReport(module_info);
}

void OpenTelemetryMetricsClientFilter::ReportClientHandledSeconds(const ClientContextPtr& context,
                                                                  ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kClientHandledTime;
  module_info.cost_time = (trpc::time::GetMicroSeconds() - context->GetSendTimestampUs()) / 1000;
  metrics_plugin_->ModuleReport(module_info);
}

void OpenTelemetryMetricsClientFilter::ReportClientHandledTotal(const ClientContextPtr& context,
                                                                ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kClientHandledCount;
  int ret_code = context->GetStatus().GetFrameworkRetCode();
  if (ret_code == trpc::TrpcRetCode::TRPC_INVOKE_SUCCESS) {
    ret_code = context->GetStatus().GetFuncRetCode();
  }

  trpc::opentelemetry::SetCallResult(ret_code, context->GetCalleeName(), context->GetFuncName(), module_info.infos);
  metrics_plugin_->ModuleReport(module_info);
}

}  // namespace trpc

#endif
