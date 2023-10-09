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

#include "examples/proxy/forward_service.h"

#include <random>

#include "trpc/client/client_context.h"
#include "trpc/client/make_client_context.h"
#include "trpc/client/trpc_client.h"
#include "trpc/log/trpc_log.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

namespace examples::forward {

ForwardServiceImpl::ForwardServiceImpl() {
  greeter_proxy_ =
      ::trpc::GetTrpcClient()->GetProxy<::trpc::test::helloworld::GreeterServiceProxy>("trpc.test.helloworld.Greeter");
}

::trpc::Status ForwardServiceImpl::Route(::trpc::ServerContextPtr context,
                                         const ::trpc::test::helloworld::HelloRequest* request,
                                         ::trpc::test::helloworld::HelloReply* reply) {
  TRPC_FMT_INFO("Forward request:{}, req id:{}", request->msg(), context->GetRequestId());

  // 1 uses the tracing interface.
  // 1.1 gets the current span and performs some operation provided by the OpenTelemetry API
  ::trpc::opentelemetry::OpenTelemetryTracingSpanPtr span = ::trpc::opentelemetry::GetTracingSpan(context);
  if (span.get()) {
    TRPC_FMT_INFO("Add info to OpenTelemetry span");
    span->SetAttribute("spankey", "spanvalue");
    span->AddEvent("eventkey", {{"int value", 8888}, {"string value", "test"}});
  }
  // 1.2 gets the trace id and span id
  TRPC_FMT_INFO("the OpenTelemetry trace id is {}", ::trpc::opentelemetry::GetTraceID(context));
  TRPC_FMT_INFO("the OpenTelemetry span id is {}", ::trpc::opentelemetry::GetSpanID(context));

  // 2 uses the metrics interface.
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  std::random_device rd;
  std::mt19937 eng(rd());
  std::uniform_int_distribution<> distr(0, 99);
  int random_num = distr(eng);
  TRPC_FMT_INFO("Add random number {} to OpenTelemetry metrics", random_num);

  ::trpc::opentelemetry::ReportSumMetricsInfo(
      {{"sum_test_key1", "sum_test_value1"}, {"sum_test_key2", "sum_test_value2"}}, random_num);
  ::trpc::opentelemetry::ReportSetMetricsInfo(
      {{"set_test_key1", "set_test_value1"}, {"set_test_key2", "set_test_value2"}}, random_num);
  ::trpc::opentelemetry::ReportMidMetricsInfo(
      {{"mid_test_key1", "mid_test_value1"}, {"mid_test_key2", "mid_test_value2"}}, random_num);
  ::trpc::opentelemetry::ReportQuantilesMetricsInfo(
      {{"quantiles_test_key1", "quantiles_test_value1"}, {"quantiles_test_key2", "quantiles_test_value2"}},
      {{0.5, 0.05}, {0.1, 0.05}}, random_num);
  ::trpc::opentelemetry::ReportHistogramMetricsInfo(
      {{"histogram_test_key1", "histogram_test_value1"}, {"histogram_test_key2", "histogram_test_value2"}},
      {10, 20, 50}, random_num);
#endif

  // 3 prints OpenTelemetry logs
#ifdef ENABLE_LOGS_PREVIEW
  TRPC_LOGGER_FMT_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg: {}", request->msg());
#endif

  // uses the ServerContext to construct ClientContext so that the entire call chain can be linked together.
  auto client_context = ::trpc::MakeClientContext(context, greeter_proxy_);

  ::trpc::test::helloworld::HelloRequest route_request;
  route_request.set_msg(request->msg());
  ::trpc::test::helloworld::HelloReply route_reply;
  // block current fiber, not block current fiber worker thread
  ::trpc::Status status = greeter_proxy_->SayHello(client_context, route_request, &route_reply);

  TRPC_FMT_INFO("Forward status:{}, route_reply:{}", status.ToString(), route_reply.msg());

  reply->set_msg(route_reply.msg());

  return status;
}

}  // namespace examples::forward
