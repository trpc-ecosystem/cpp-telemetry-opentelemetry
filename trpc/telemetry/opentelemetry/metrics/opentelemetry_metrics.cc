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
#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics.h"

#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"

namespace trpc {

int OpenTelemetryMetrics::Init() noexcept {
  bool ret = TrpcConfig::GetInstance()->GetPluginConfig("telemetry", trpc::opentelemetry::kOpenTelemetryTelemetryName,
                                                        config_);
  if (!ret) {
    TRPC_LOG_ERROR("get opentelemetry config fail, ret:" << ret);
    return -1;
  }

  if (!config_.metrics_config.enabled) {  // does not enable metrics
    TRPC_LOG_DEBUG("opentelemetry do not enable metrics, no need init");
    return 0;
  }

  // initialize metrics family
  client_started_total_family_ = trpc::prometheus::GetCounterFamily(kClientStartedTotalName, kClientStartedTotalDesc);
  client_handled_total_family_ = trpc::prometheus::GetCounterFamily(kClientHandledTotalName, kClientHandledTotalDesc);
  client_handled_seconds_family_ =
      trpc::prometheus::GetHistogramFamily(kClientHandledSecondsName, kClientHandledSecondsDesc);
  server_started_total_family_ = trpc::prometheus::GetCounterFamily(kServerStartedTotalName, kServerStartedTotalDesc);
  server_handled_total_family_ = trpc::prometheus::GetCounterFamily(kServerHandledTotalName, kServerHandledTotalDesc);
  server_handled_seconds_family_ =
      trpc::prometheus::GetHistogramFamily(kServerHandledSecondsName, kServerHandledSecondsDesc);
  opentelemetry_counter_family_ =
      trpc::prometheus::GetCounterFamily(kOpenTelemetryCounterName, kOpenTelemetryCounterDesc);
  opentelemetry_gauge_family_ = trpc::prometheus::GetGaugeFamily(kOpenTelemetryGaugeName, kOpenTelemetryGaugeDesc);
  opentelemetry_summary_family_ =
      trpc::prometheus::GetSummaryFamily(kOpenTelemetrySummaryName, kOpenTelemetrySummaryDesc);
  opentelemetry_histogram_family_ =
      trpc::prometheus::GetHistogramFamily(kOpenTelemetryHistogramName, kOpenTelemetryHistogramDesc);

  // initializes the map of ModuleReportFunc for different ModuleReportType
  module_report_map_[trpc::opentelemetry::ModuleReportType::kClientStartedCount] =
      [this](const ModuleMetricsInfo& info) { ClientStartedTotalReportFunc(info); };
  module_report_map_[trpc::opentelemetry::ModuleReportType::kClientHandledCount] =
      [this](const ModuleMetricsInfo& info) { ClientHandledTotalReportFunc(info); };
  module_report_map_[trpc::opentelemetry::ModuleReportType::kClientHandledTime] =
      [this](const ModuleMetricsInfo& info) { ClientHandledSecondsReportFunc(info); };
  module_report_map_[trpc::opentelemetry::ModuleReportType::kServerStartedCount] =
      [this](const ModuleMetricsInfo& info) { ServerStartedTotalReportFunc(info); };
  module_report_map_[trpc::opentelemetry::ModuleReportType::kServerHandledCount] =
      [this](const ModuleMetricsInfo& info) { ServerHandledTotalReportFunc(info); };
  module_report_map_[trpc::opentelemetry::ModuleReportType::kServerHandledTime] =
      [this](const ModuleMetricsInfo& info) { ServerHandledSecondsReportFunc(info); };

  // initializes the error code mapping map
  trpc::opentelemetry::InitUserCodeMap(config_.metrics_config.codes);
  trpc::opentelemetry::InitDefaultCodeMap();

  return 0;
}

int OpenTelemetryMetrics::ModuleReport(const ModuleMetricsInfo& info) {
  if (!config_.metrics_config.enabled) {  // does not enable metrics
    TRPC_LOG_DEBUG("opentelemetry do not enable metrics, can not report");
    return -1;
  }

  if (info.extend_info.type() != typeid(trpc::opentelemetry::ModuleReportType)) {
    TRPC_LOG_ERROR("extend_info type is invalid");
    return -1;
  }

  auto type = std::any_cast<trpc::opentelemetry::ModuleReportType>(info.extend_info);
  if (module_report_map_.find(type) != module_report_map_.end()) {
    module_report_map_[type](info);
    return 0;
  }
  TRPC_LOG_ERROR("unknown prometheus type: " << static_cast<int>(type));
  return -1;
}

void OpenTelemetryMetrics::ClientStartedTotalReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_counter = client_started_total_family_->Add(metrics_info.infos);
  module_counter.Increment();
}

void OpenTelemetryMetrics::ClientHandledTotalReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_counter = client_handled_total_family_->Add(metrics_info.infos);
  module_counter.Increment();
}

void OpenTelemetryMetrics::ClientHandledSecondsReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_histogram =
      client_handled_seconds_family_->Add(metrics_info.infos, config_.metrics_config.client_histogram_buckets);
  module_histogram.Observe(static_cast<double>(metrics_info.cost_time) / 1000);
}

void OpenTelemetryMetrics::ServerStartedTotalReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_counter = server_started_total_family_->Add(metrics_info.infos);
  module_counter.Increment();
}

void OpenTelemetryMetrics::ServerHandledTotalReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_counter = server_handled_total_family_->Add(metrics_info.infos);
  module_counter.Increment();
}

void OpenTelemetryMetrics::ServerHandledSecondsReportFunc(const ModuleMetricsInfo& metrics_info) {
  auto& module_histogram =
      server_handled_seconds_family_->Add(metrics_info.infos, config_.metrics_config.server_histogram_buckets);
  module_histogram.Observe(static_cast<double>(metrics_info.cost_time) / 1000);
}

int OpenTelemetryMetrics::SetDataReport(const std::map<std::string, std::string>& labels, double value) {
  auto& gauge = opentelemetry_gauge_family_->Add(labels);
  gauge.Set(value);
  return 0;
}

int OpenTelemetryMetrics::SumDataReport(const std::map<std::string, std::string>& labels, double value) {
  auto& counter = opentelemetry_counter_family_->Add(labels);
  counter.Increment(value);
  return 0;
}

int OpenTelemetryMetrics::MidDataReport(const std::map<std::string, std::string>& labels, double value) {
  auto pro_quantiles = ::prometheus::Summary::Quantiles{{0.5, 0.05}};
  auto& summary = opentelemetry_summary_family_->Add(labels, std::move(pro_quantiles));
  summary.Observe(value);
  return 0;
}

int OpenTelemetryMetrics::QuantilesDataReport(const std::map<std::string, std::string>& labels,
                                              const SummaryQuantiles& quantiles, double value) {
  if (quantiles.size() == 0) {
    TRPC_LOG_ERROR("quantiles size must > 0");
    return -1;
  }
  ::prometheus::Summary::Quantiles pro_quantiles;
  for (auto val : quantiles) {
    if (val.size() != 2) {
      TRPC_LOG_ERROR("each value in quantiles must have a size of 2");
      return -1;
    }
    pro_quantiles.emplace_back(::prometheus::detail::CKMSQuantiles::Quantile(val[0], val[1]));
  }
  auto& summary = opentelemetry_summary_family_->Add(labels, std::move(pro_quantiles));
  summary.Observe(value);
  return 0;
}

namespace {

template <typename T>
int HistogramDataReportTemplate(::prometheus::Family<::prometheus::Histogram>* family,
                                const std::map<std::string, std::string>& labels, T&& bucket, double value) {
  if (bucket.size() == 0) {
    TRPC_LOG_ERROR("bucket size must > 0");
    return -1;
  }
  auto& histogram = family->Add(labels, std::forward<T>(bucket));
  histogram.Observe(value);
  return 0;
}

}  // namespace

int OpenTelemetryMetrics::HistogramDataReport(const std::map<std::string, std::string>& labels,
                                              HistogramBucket&& bucket, double value) {
  return HistogramDataReportTemplate(opentelemetry_histogram_family_, labels, std::move(bucket), value);
}

int OpenTelemetryMetrics::HistogramDataReport(const std::map<std::string, std::string>& labels,
                                              const HistogramBucket& bucket, double value) {
  return HistogramDataReportTemplate(opentelemetry_histogram_family_, labels, bucket, value);
}

int OpenTelemetryMetrics::SingleAttrReport(const SingleAttrMetricsInfo& info) { return SingleAttrReportTemplate(info); }

int OpenTelemetryMetrics::SingleAttrReport(SingleAttrMetricsInfo&& info) {
  return SingleAttrReportTemplate(std::move(info));
}

int OpenTelemetryMetrics::MultiAttrReport(const MultiAttrMetricsInfo& info) { return MultiAttrReportTemplate(info); }

int OpenTelemetryMetrics::MultiAttrReport(MultiAttrMetricsInfo&& info) {
  return MultiAttrReportTemplate(std::move(info));
}

}  // namespace trpc
#endif
