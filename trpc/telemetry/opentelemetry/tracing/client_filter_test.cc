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

#include "trpc/telemetry/opentelemetry/tracing/client_filter.h"

#include "gtest/gtest.h"
#include "opentelemetry/trace/tracer.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/codec/http/http_client_codec.h"
#include "trpc/codec/trpc/trpc_client_codec.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"

namespace trpc::testing {

class TracingClientFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);
    RegisterPlugins();

    // registers a telemetry to ensure that OpenTelemetryTracingClientFilter initializes success
    auto telemetry = MakeRefCounted<MockOpenTelemetryTelemetry>();
    tracing_ = MakeRefCounted<OpenTelemetryTracing>();
    ASSERT_EQ(0, tracing_->Init());
    TelemetryFactory::GetInstance()->Register(telemetry);
    EXPECT_CALL(*telemetry, GetTracing()).WillOnce(::testing::Return(tracing_));
    client_filter_ = std::make_shared<OpenTelemetryTracingClientFilter>();
    ASSERT_EQ(0, client_filter_->Init());
  }

  static void TearDownTestCase() { UnregisterPlugins(); }

  static ClientContextPtr GetTestTrpcClientContext() {
    auto trpc_codec = std::make_shared<trpc::TrpcClientCodec>();
    ClientContextPtr context = MakeRefCounted<ClientContext>(trpc_codec);
    context->SetCallerName("trpc.test.helloworld.client");
    return context;
  }

  static ClientContextPtr GetTestHttpClientContext() {
    auto http_codec = std::make_shared<trpc::HttpClientCodec>();
    ClientContextPtr context = MakeRefCounted<ClientContext>(http_codec);
    context->SetCallerName("trpc.test.helloworld.client");
    return context;
  }

 protected:
  static MessageClientFilterPtr client_filter_;
  static OpenTelemetryTracingPtr tracing_;
};

MessageClientFilterPtr TracingClientFilterTest::client_filter_;
OpenTelemetryTracingPtr TracingClientFilterTest::tracing_;

class TestCarrier : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override {
    return "";
  }

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override {}
};

trpc::opentelemetry::TextMapCarrierPtr ClientTestCarrierFunc(const trpc::ClientContextPtr& context) {
  return std::make_unique<TestCarrier>();
}

TEST_F(TracingClientFilterTest, ClientCarrier) {
  // for protocols that are not set, the default implementation will be returned
  ASSERT_NE(nullptr, trpc::opentelemetry::GetClientCarrierFunc("not_set_protocol"));

  // testing for setter and getter
  trpc::opentelemetry::SetClientCarrierFunc("test_protocol", ClientTestCarrierFunc);
  ASSERT_NE(nullptr, trpc::opentelemetry::GetClientCarrierFunc("test_protocol"));
  trpc::opentelemetry::SetClientCarrierFunc("test_protocol", nullptr);
  ASSERT_NE(nullptr, trpc::opentelemetry::GetClientCarrierFunc("test_protocol"));

  // testing for ClientTransInfoCarrierFunc
  ClientContextPtr trpc_context = GetTestTrpcClientContext();
  ASSERT_NE(nullptr, trpc::opentelemetry::ClientTransInfoCarrierFunc(trpc_context));

  // testing for ClientHttpCarrierFunc
  ClientContextPtr http_context = GetTestHttpClientContext();
  ASSERT_NE(nullptr, trpc::opentelemetry::ClientHttpCarrierFunc(http_context));
}

TEST_F(TracingClientFilterTest, Init) {
  ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, client_filter_->Name());
  std::vector<FilterPoint> points = client_filter_->GetFilterPoint();
  ASSERT_EQ(2, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::CLIENT_POST_RPC_INVOKE) != points.end());
}

TEST_F(TracingClientFilterTest, TraceWithoutParentSpan) {
  ClientContextPtr context = GetTestTrpcClientContext();
  // There is no parent span in ClientTracingSpan.
  context->SetFilterData(tracing_->GetPluginID(), ClientTracingSpan());

  // test for CLIENT_PRE_RPC_INVOKE filter point
  FilterStatus status;
  client_filter_->operator()(status, FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  ClientTracingSpan* ptr = context->GetFilterData<ClientTracingSpan>(tracing_->GetPluginID());
  ASSERT_NE(nullptr, ptr);
  ASSERT_EQ(typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr), ptr->span.type());
  auto& span = std::any_cast<trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(ptr->span);
  ASSERT_NE(nullptr, span.get());
  // it is in the recording state before reporting
  ASSERT_TRUE(span->IsRecording());

  // test for CLIENT_POST_RPC_INVOKE filter point
  trpc::Status frame_status;
  frame_status.SetFrameworkRetCode(101);
  frame_status.SetErrorMessage("timeout");
  context->SetStatus(frame_status);
  client_filter_->operator()(status, FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  // after reporting, it is in a non-recording state
  ASSERT_FALSE(span->IsRecording());
}

TEST_F(TracingClientFilterTest, TraceWithParentSpan) {
  ClientContextPtr context = GetTestTrpcClientContext();
  // There has parent span in ClientTracingSpan.
  std::string err_msg;
  auto opentelemetry_tracer = tracing_->MakeTracer(context->GetCallerName().c_str(), err_msg);
  ASSERT_TRUE(opentelemetry_tracer);
  // creates a parent span and save its trace id and span id
  auto parent_span =
      opentelemetry_tracer->StartSpan(context->GetFuncName(), {}, ::opentelemetry::trace::StartSpanOptions{});
  ASSERT_NE(nullptr, parent_span);
  char parent_trace_id[32];
  parent_span->GetContext().trace_id().ToLowerBase16(parent_trace_id);
  char parent_span_id[16];
  parent_span->GetContext().span_id().ToLowerBase16(parent_span_id);
  ClientTracingSpan client_span;
  client_span.parent_span = std::move(trpc::opentelemetry::OpenTelemetryTracingSpanPtr(std::move(parent_span)));
  context->SetFilterData(tracing_->GetPluginID(), std::move(client_span));

  // test for CLIENT_PRE_RPC_INVOKE filter point
  FilterStatus status;
  client_filter_->operator()(status, FilterPoint::CLIENT_PRE_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  ClientTracingSpan* ptr = context->GetFilterData<ClientTracingSpan>(tracing_->GetPluginID());
  ASSERT_NE(nullptr, ptr);
  ASSERT_EQ(typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr), ptr->span.type());
  auto& span = std::any_cast<trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(ptr->span);
  ASSERT_NE(nullptr, span.get());
  // it is in the recording state before reporting
  ASSERT_TRUE(span->IsRecording());

  // the trace id of the current span should be equal to the parent_trace_id,
  // the span id of the current should be a newly generated ID
  char trace_id[32];
  span->GetContext().trace_id().ToLowerBase16(trace_id);
  ASSERT_EQ(std::string(parent_trace_id, sizeof(parent_trace_id)), std::string(trace_id, sizeof(trace_id)));
  char span_id[16];
  span->GetContext().span_id().ToLowerBase16(span_id);
  ASSERT_NE(std::string(parent_span_id, sizeof(parent_span_id)), std::string(span_id, sizeof(span_id)));

  // test for CLIENT_POST_RPC_INVOKE filter point
  trpc::Status frame_status;
  frame_status.SetFrameworkRetCode(101);
  frame_status.SetErrorMessage("timeout");
  context->SetStatus(frame_status);
  client_filter_->operator()(status, FilterPoint::CLIENT_POST_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  // after reporting, it is in a non-recording state
  ASSERT_FALSE(span->IsRecording());
}

}  // namespace trpc::testing
