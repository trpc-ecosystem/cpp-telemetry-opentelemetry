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

#include "trpc/telemetry/opentelemetry/opentelemetry_client_filter.h"

namespace trpc {

OpenTelemetryClientFilter::OpenTelemetryClientFilter() {
  tracing_filter_ = std::make_shared<OpenTelemetryTracingClientFilter>();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_filter_ = std::make_shared<OpenTelemetryMetricsClientFilter>();
#endif
}

int OpenTelemetryClientFilter::Init() {
  int ret = tracing_filter_->Init();
  if (ret != 0) {
    return ret;
  }
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  ret = metrics_filter_->Init();
  if (ret != 0) {
    return ret;
  }
#endif
  return 0;
}

std::vector<FilterPoint> OpenTelemetryClientFilter::GetFilterPoint() {
  std::vector<FilterPoint> points = {FilterPoint::CLIENT_PRE_RPC_INVOKE, FilterPoint::CLIENT_POST_RPC_INVOKE};
  return points;
}

void OpenTelemetryClientFilter::operator()(FilterStatus& status, FilterPoint point, const ClientContextPtr& context) {
  if (point == FilterPoint::CLIENT_PRE_RPC_INVOKE || point == FilterPoint::CLIENT_POST_RPC_INVOKE) {
    tracing_filter_->operator()(status, point, context);
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
    metrics_filter_->operator()(status, point, context);
#endif
  }
  status = FilterStatus::CONTINUE;
}

}  // namespace trpc
