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

#include <functional>
#include <map>
#include <unordered_map>
#include <vector>

#include "prometheus/counter.h"
#include "prometheus/gauge.h"
#include "prometheus/histogram.h"
#include "prometheus/registry.h"
#include "prometheus/summary.h"
#include "trpc/metrics/metrics.h"
#include "trpc/util/log/logging.h"
#include "trpc/util/prometheus.h"

#include "trpc/telemetry/opentelemetry/metrics/common.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"

namespace trpc {

/// @brief The implementation class for metrics capabilities of the OpenTelemetry plugin.
class OpenTelemetryMetrics : public Metrics {
 public:
  std::string Name() const override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  int Init() noexcept override;

  int ModuleReport(const ModuleMetricsInfo& info) override;

  int SingleAttrReport(const SingleAttrMetricsInfo& info) override;
  int SingleAttrReport(SingleAttrMetricsInfo&& info) override;

  int MultiAttrReport(const MultiAttrMetricsInfo& info) override;
  int MultiAttrReport(MultiAttrMetricsInfo&& info) override;

  /// @brief Gets the config for OpenTelemetryMetrics
  const OpenTelemetryConfig& GetConfig() { return config_; }

  /// @brief Reports metrics data with SET type
  /// @note This interface is for internal use only and should not be used by users. May be modified in the future.
  int SetDataReport(const std::map<std::string, std::string>& labels, double value);

  /// @brief Reports metrics data with SUM type
  /// @note This interface is for internal use only and should not be used by users. May be modified in the future.
  int SumDataReport(const std::map<std::string, std::string>& labels, double value);

  /// @brief Reports metrics data with MID type
  /// @note This interface is for internal use only and should not be used by users. May be modified in the future.
  int MidDataReport(const std::map<std::string, std::string>& labels, double value);

  /// @brief Reports metrics data with QUANTILES type
  /// @note This interface is for internal use only and should not be used by users. May be modified in the future.
  int QuantilesDataReport(const std::map<std::string, std::string>& labels, const SummaryQuantiles& quantiles,
                          double value);

  /// @brief Reports metrics data with HISTOGRAM type
  /// @note This interface is for internal use only and should not be used by users. May be modified in the future.
  int HistogramDataReport(const std::map<std::string, std::string>& labels, HistogramBucket&& bucket, double value);
  int HistogramDataReport(const std::map<std::string, std::string>& labels, const HistogramBucket& bucket,
                          double value);

 private:
  // Type definition of reporting functions for module metrics data
  using ModuleReportFunc = std::function<void(const ModuleMetricsInfo& info)>;

  // ModuleReportFunc for kClientStartedCount type
  void ClientStartedTotalReportFunc(const ModuleMetricsInfo& metrics_info);

  // ModuleReportFunc for kClientHandledCount type
  void ClientHandledTotalReportFunc(const ModuleMetricsInfo& metrics_info);

  // ModuleReportFunc for kClientHandledTime type
  void ClientHandledSecondsReportFunc(const ModuleMetricsInfo& metrics_info);

  // ModuleReportFunc for kServerStartedCount type
  void ServerStartedTotalReportFunc(const ModuleMetricsInfo& metrics_info);

  // ModuleReportFunc for kServerHandledCount type
  void ServerHandledTotalReportFunc(const ModuleMetricsInfo& metrics_info);

  // ModuleReportFunc for kServerHandledTime type
  void ServerHandledSecondsReportFunc(const ModuleMetricsInfo& metrics_info);

  template <typename T>
  int SingleAttrReportTemplate(T&& info) {
    if (!config_.metrics_config.enabled) {  // does not enable metrics
      TRPC_LOG_DEBUG("opentelemetry do not enable metrics, can not report");
      return -1;
    }

    std::map<std::string, std::string> labels = {{std::forward<T>(info).name, std::forward<T>(info).dimension}};
    switch (info.policy) {
      case SET:
        return SetDataReport(labels, info.value);
      case SUM:
        return SumDataReport(labels, info.value);
      case MID:
        return MidDataReport(labels, info.value);
      case QUANTILES:
        return QuantilesDataReport(labels, info.quantiles, info.value);
      case HISTOGRAM:
        return HistogramDataReport(labels, std::forward<T>(info).bucket, info.value);
      default:
        TRPC_LOG_ERROR("unknown policy type: " << static_cast<int>(info.policy));
    }
    return -1;
  }

  template <typename T>
  int MultiAttrReportTemplate(T&& info) {
    if (!config_.metrics_config.enabled) {  // does not enable metrics
      TRPC_LOG_DEBUG("opentelemetry do not enable metrics, can not report");
      return -1;
    }

    if (info.values.empty()) {
      TRPC_LOG_ERROR("values can not be empty");
      return -1;
    }

    MetricsPolicy policy = info.values[0].first;
    double value = info.values[0].second;
    switch (policy) {
      case SET:
        return SetDataReport(info.tags, value);
      case SUM:
        return SumDataReport(info.tags, value);
      case MID:
        return MidDataReport(info.tags, value);
      case QUANTILES:
        return QuantilesDataReport(info.tags, info.quantiles, value);
      case HISTOGRAM:
        return HistogramDataReport(info.tags, std::forward<T>(info).bucket, value);
      default:
        TRPC_LOG_ERROR("unknown policy type: " << static_cast<int>(policy));
    }
    return -1;
  }

 private:
  OpenTelemetryConfig config_;

  // Map of ModuleReportFunc for different ModuleReportType
  std::unordered_map<trpc::opentelemetry::ModuleReportType, ModuleReportFunc> module_report_map_;

  // metrics family for number of client-side RPC calls
  ::prometheus::Family<::prometheus::Counter>* client_started_total_family_;
  static constexpr char kClientStartedTotalName[] = "rpc_client_started_total";
  static constexpr char kClientStartedTotalDesc[] = "Total number of RPCs started on the client.";
  // metrics family for for client requests, error rate, timeout rate, and success rate
  ::prometheus::Family<::prometheus::Counter>* client_handled_total_family_;
  static constexpr char kClientHandledTotalName[] = "rpc_client_handled_total";
  static constexpr char kClientHandledTotalDesc[] =
      "Total number of RPCs completed by the client, regardless of success or failure.";
  // metrics family for distribution of client-side execution time
  ::prometheus::Family<::prometheus::Histogram>* client_handled_seconds_family_;
  static constexpr char kClientHandledSecondsName[] = "rpc_client_handled_seconds";
  static constexpr char kClientHandledSecondsDesc[] =
      "Histogram of response latency (seconds) of the RPC until it is finished by the application.";

  // metrics family for number of server-side RPC calls
  ::prometheus::Family<::prometheus::Counter>* server_started_total_family_;
  static constexpr char kServerStartedTotalName[] = "rpc_server_started_total";
  static constexpr char kServerStartedTotalDesc[] = "Total number of RPCs started on the server.";
  // metrics family for for server requests, error rate, timeout rate, and success rate
  ::prometheus::Family<::prometheus::Counter>* server_handled_total_family_;
  static constexpr char kServerHandledTotalName[] = "rpc_server_handled_total";
  static constexpr char kServerHandledTotalDesc[] =
      "Total number of RPCs completed on the server, regardless of success or failure.";
  // metrics family for distribution of server-side execution time
  ::prometheus::Family<::prometheus::Histogram>* server_handled_seconds_family_;
  static constexpr char kServerHandledSecondsName[] = "rpc_server_handled_seconds";
  static constexpr char kServerHandledSecondsDesc[] =
      "Histogram of response latency (seconds) of RPC that had been application-level handled by the server.";

  // custom metrics data
  ::prometheus::Family<::prometheus::Counter>* opentelemetry_counter_family_;
  static constexpr char kOpenTelemetryCounterName[] = "opentelemetry_counter_report";
  static constexpr char kOpenTelemetryCounterDesc[] = "trpc-cpp opentelemetry counter report.";
  ::prometheus::Family<::prometheus::Gauge>* opentelemetry_gauge_family_;
  static constexpr char kOpenTelemetryGaugeName[] = "opentelemetry_gauge_report";
  static constexpr char kOpenTelemetryGaugeDesc[] = "trpc-cpp opentelemetry gauge report.";
  ::prometheus::Family<::prometheus::Summary>* opentelemetry_summary_family_;
  static constexpr char kOpenTelemetrySummaryName[] = "opentelemetry_summary_report";
  static constexpr char kOpenTelemetrySummaryDesc[] = "trpc-cpp opentelemetry summary report.";
  ::prometheus::Family<::prometheus::Histogram>* opentelemetry_histogram_family_;
  static constexpr char kOpenTelemetryHistogramName[] = "opentelemetry_histogram_report";
  static constexpr char kOpenTelemetryHistogramDesc[] = "trpc-cpp opentelemetry histogram report.";
};

using OpenTelemetryMetricsPtr = RefPtr<OpenTelemetryMetrics>;

}  // namespace trpc
#endif
