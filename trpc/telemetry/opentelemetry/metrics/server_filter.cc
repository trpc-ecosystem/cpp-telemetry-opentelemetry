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
#include "trpc/telemetry/opentelemetry/metrics/server_filter.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/util/time.h"

#include "trpc/telemetry/opentelemetry/metrics/common.h"

namespace trpc {

int OpenTelemetryMetricsServerFilter::Init() {
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

std::vector<FilterPoint> OpenTelemetryMetricsServerFilter::GetFilterPoint() {
  std::vector<FilterPoint> points = {FilterPoint::SERVER_POST_RECV_MSG, FilterPoint::SERVER_PRE_SEND_MSG};
  return points;
}

void OpenTelemetryMetricsServerFilter::operator()(FilterStatus& status, FilterPoint point,
                                                  const ServerContextPtr& context) {
  status = FilterStatus::CONTINUE;
  if (!enabled_) {
    return;
  }

  ModuleMetricsInfo module_info;
  module_info.source = kMetricsCalleeSource;
  module_info.infos[trpc::opentelemetry::kCallerService] = context->GetCallerName();
  module_info.infos[trpc::opentelemetry::kCallerMethod] = "";
  module_info.infos[trpc::opentelemetry::kCalleeService] = context->GetCalleeName();
  module_info.infos[trpc::opentelemetry::kCalleeMethod] = context->GetFuncName();

  if (point == FilterPoint::SERVER_POST_RECV_MSG) {
    ReportServerStartedTotal(context, module_info);
  } else if (point == FilterPoint::SERVER_PRE_SEND_MSG) {
    ReportServerHandledSeconds(context, module_info);
    ReportServerHandledTotal(context, module_info);
  }
}

void OpenTelemetryMetricsServerFilter::ReportServerStartedTotal(const ServerContextPtr& context,
                                                                ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kServerStartedCount;
  metrics_plugin_->ModuleReport(module_info);
}

void OpenTelemetryMetricsServerFilter::ReportServerHandledSeconds(const ServerContextPtr& context,
                                                                  ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kServerHandledTime;
  module_info.cost_time = trpc::time::GetMilliSeconds() - context->GetRecvTimestamp();
  metrics_plugin_->ModuleReport(module_info);
}

void OpenTelemetryMetricsServerFilter::ReportServerHandledTotal(const ServerContextPtr& context,
                                                                ModuleMetricsInfo& module_info) {
  module_info.extend_info = trpc::opentelemetry::ModuleReportType::kServerHandledCount;
  int ret_code = context->GetStatus().GetFrameworkRetCode();
  if (ret_code == trpc::TrpcRetCode::TRPC_INVOKE_SUCCESS) {
    ret_code = context->GetStatus().GetFuncRetCode();
  }

  trpc::opentelemetry::SetCallResult(ret_code, context->GetCalleeName(), context->GetFuncName(), module_info.infos);
  metrics_plugin_->ModuleReport(module_info);
}

}  // namespace trpc

#endif
