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

#include "trpc/telemetry/opentelemetry/opentelemetry_server_filter.h"

namespace trpc {

OpenTelemetryServerFilter::OpenTelemetryServerFilter() {
  tracing_filter_ = std::make_shared<OpenTelemetryTracingServerFilter>();
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  metrics_filter_ = std::make_shared<OpenTelemetryMetricsServerFilter>();
#endif
}

int OpenTelemetryServerFilter::Init() {
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

std::vector<FilterPoint> OpenTelemetryServerFilter::GetFilterPoint() {
  // the filter points of tracing
  std::vector<FilterPoint> points = {FilterPoint::SERVER_PRE_RPC_INVOKE, FilterPoint::SERVER_POST_RPC_INVOKE};
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  // the filter points of metrics
  points.push_back(FilterPoint::SERVER_POST_RECV_MSG);
  points.push_back(FilterPoint::SERVER_PRE_SEND_MSG);
#endif
  return points;
}

void OpenTelemetryServerFilter::operator()(FilterStatus& status, FilterPoint point, const ServerContextPtr& context) {
  // deals with tracing filter point
  if (point == FilterPoint::SERVER_PRE_RPC_INVOKE || point == FilterPoint::SERVER_POST_RPC_INVOKE) {
    tracing_filter_->operator()(status, point, context);
  }
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  // deals with metrics filter point
  if (point == FilterPoint::SERVER_POST_RECV_MSG || point == FilterPoint::SERVER_PRE_SEND_MSG) {
    metrics_filter_->operator()(status, point, context);
  }
#endif
  status = FilterStatus::CONTINUE;
}

}  // namespace trpc
