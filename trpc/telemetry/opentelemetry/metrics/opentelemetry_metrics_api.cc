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
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics_api.h"

#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/util/log/logging.h"

#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"

namespace trpc::opentelemetry {

namespace {

OpenTelemetryMetricsPtr GetMetricsPlugin() {
  TelemetryPtr telemetry = TelemetryFactory::GetInstance()->Get(trpc::opentelemetry::kOpenTelemetryTelemetryName);
  if (!telemetry) {
    TRPC_FMT_DEBUG("get telemetry plugin {} failed.", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return nullptr;
  }
  OpenTelemetryMetricsPtr metrics = trpc::dynamic_pointer_cast<trpc::OpenTelemetryMetrics>(telemetry->GetMetrics());
  return metrics;
}

template <typename T>
int ReportHistogramTemplate(const std::map<std::string, std::string>& labels, T&& bucket, double value) {
  trpc::OpenTelemetryMetricsPtr metrics = GetMetricsPlugin();
  if (!metrics) {
    return -1;
  }
  return metrics->HistogramDataReport(labels, std::forward<T>(bucket), value);
}

}  // namespace

int ReportSumMetricsInfo(const std::map<std::string, std::string>& labels, double value) {
  trpc::OpenTelemetryMetricsPtr metrics = GetMetricsPlugin();
  if (!metrics) {
    return -1;
  }
  return metrics->SumDataReport(labels, value);
}

int ReportSetMetricsInfo(const std::map<std::string, std::string>& labels, double value) {
  trpc::OpenTelemetryMetricsPtr metrics = GetMetricsPlugin();
  if (!metrics) {
    return -1;
  }
  return metrics->SetDataReport(labels, value);
}

int ReportMidMetricsInfo(const std::map<std::string, std::string>& labels, double value) {
  trpc::OpenTelemetryMetricsPtr metrics = GetMetricsPlugin();
  if (!metrics) {
    return -1;
  }
  return metrics->MidDataReport(labels, value);
}

int ReportQuantilesMetricsInfo(const std::map<std::string, std::string>& labels, const SummaryQuantiles& quantiles,
                               double value) {
  trpc::OpenTelemetryMetricsPtr metrics = GetMetricsPlugin();
  if (!metrics) {
    return -1;
  }
  return metrics->QuantilesDataReport(labels, quantiles, value);
}

int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, const HistogramBucket& bucket,
                               double value) {
  return ReportHistogramTemplate(labels, bucket, value);
}

int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, HistogramBucket&& bucket,
                               double value) {
  return ReportHistogramTemplate(labels, std::move(bucket), value);
}

}  // namespace trpc::opentelemetry
#endif
