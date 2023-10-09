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

#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"

#include "opentelemetry/exporters/otlp/otlp_http_exporter.h"
#include "opentelemetry/sdk/trace/batch_span_processor.h"
#include "opentelemetry/sdk/trace/tracer_provider.h"
#include "opentelemetry/trace/provider.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/util/log/logging.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf_parser.h"
#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/telemetry/opentelemetry/tracing/deferred_sample_processor.h"
#include "trpc/telemetry/opentelemetry/tracing/grpc_trace_exporter.h"
#include "trpc/telemetry/opentelemetry/tracing/sampler.h"

namespace trpc {

int OpenTelemetryTracing::Init() noexcept {
  bool ret = TrpcConfig::GetInstance()->GetPluginConfig("telemetry", trpc::opentelemetry::kOpenTelemetryTelemetryName,
                                                        config_);
  if (!ret) {
    TRPC_FMT_ERROR("get {} config fail!", trpc::opentelemetry::kOpenTelemetryTelemetryName);
    return -1;
  }

  if (!InitOpenTelemetry()) {
    TRPC_LOG_ERROR("InitOpenTelemetry failed...");
    return -1;
  }
  return 0;
}

std::unique_ptr<::opentelemetry::sdk::trace::SpanExporter> OpenTelemetryTracing::GetExporter() {
  if (config_.protocol == "http") {
    ::opentelemetry::exporter::otlp::OtlpHttpExporterOptions exporter_opts;
    exporter_opts.url = config_.addr + "/v1/traces";
    return std::make_unique<::opentelemetry::exporter::otlp::OtlpHttpExporter>(exporter_opts);
  } else if (config_.protocol == "grpc") {
    ServiceProxyOption service_opts;
    service_opts.name = trpc::opentelemetry::kGrpcTraceExporterServiceName;
    service_opts.codec_name = config_.protocol;
    service_opts.selector_name = config_.selector_name;
    service_opts.timeout = config_.timeout;
    service_opts.target = config_.addr;
    return std::make_unique<trpc::opentelemetry::GrpcTraceExporter>(service_opts);
  }
  return nullptr;
}

bool OpenTelemetryTracing::InitOpenTelemetry() {
  // initializes exporter
  std::unique_ptr<::opentelemetry::sdk::trace::SpanExporter> exporter = GetExporter();
  if (!exporter) {
    TRPC_FMT_ERROR("get opentelemetry exporter fail, protocol is invalid: {}", config_.protocol);
    return false;
  }

  // initializes processor
  ::opentelemetry::sdk::trace::BatchSpanProcessorOptions batch_op;
  std::unique_ptr<::opentelemetry::sdk::trace::SpanProcessor> processor;
  if (config_.traces_config.enable_deferred_sample) {
    trpc::opentelemetry::DeferredSampleProcessor::Options deferred_opts;
    deferred_opts.enable_sample_error = config_.traces_config.enable_deferred_sample;
    deferred_opts.sample_slow_duration = std::chrono::milliseconds(config_.traces_config.deferred_sample_slow_duration);
    processor = std::make_unique<trpc::opentelemetry::DeferredSampleProcessor>(
        std::make_unique<::opentelemetry::sdk::trace::BatchSpanProcessor>(std::move(exporter), batch_op),
        std::move(deferred_opts));
  } else {
    processor = std::make_unique<::opentelemetry::sdk::trace::BatchSpanProcessor>(std::move(exporter), batch_op);
  }

  // initializes provider
  ::opentelemetry::sdk::common::AttributeMap resources_map = {
      {trpc::opentelemetry::kTraceServiceName,
       TrpcConfig::GetInstance()->GetServerConfig().app + "." + TrpcConfig::GetInstance()->GetServerConfig().server}};
  for (auto& [key, value] : config_.traces_config.resources) {
    resources_map.SetAttribute(key, value);
  }
  auto resource = ::opentelemetry::sdk::resource::Resource::Create(resources_map);
  trpc::opentelemetry::Sampler::Options sample_opts;
  sample_opts.ratio = config_.sampler_config.fraction;
  sample_opts.disable_parent_sampling = config_.traces_config.disable_parent_sampling;
  sample_opts.enable_deferred_sample = config_.traces_config.enable_deferred_sample;
  std::shared_ptr<::opentelemetry::trace::TracerProvider> provider =
      std::make_shared<::opentelemetry::sdk::trace::TracerProvider>(
          std::move(processor), resource, std::make_unique<trpc::opentelemetry::Sampler>(std::move(sample_opts)));
  ::opentelemetry::trace::Provider::SetTracerProvider(provider);
  return true;
}

::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Tracer> OpenTelemetryTracing::MakeTracer(
    const char* service_name, std::string& error_message) {
  auto provider = ::opentelemetry::trace::Provider::GetTracerProvider();
  return provider->GetTracer(strlen(service_name) > 0 ? service_name : "default_service");
}

}  // namespace trpc
