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

#include "trpc/telemetry/opentelemetry/tracing/server_filter.h"

#include "gtest/gtest.h"
#include "opentelemetry/trace/propagation/http_trace_context.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/codec/trpc/testing/trpc_protocol_testing.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/proto/testing/helloworld.pb.h"
#include "trpc/server/http_service.h"
#include "trpc/server/rpc/rpc_service_impl.h"
#include "trpc/server/server_context.h"
#include "trpc/server/service_adapter.h"
#include "trpc/server/testing/server_context_testing.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/tracing/tracing_filter_index.h"

#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"

namespace trpc::testing {

class TracingServerFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);
    RegisterPlugins();

    // registers a telemetry to ensure that OpenTelemetryTracingServerFilter initializes success
    auto telemetry = MakeRefCounted<MockOpenTelemetryTelemetry>();
    tracing_ = MakeRefCounted<OpenTelemetryTracing>();
    ASSERT_EQ(0, tracing_->Init());
    TelemetryFactory::GetInstance()->Register(telemetry);
    EXPECT_CALL(*telemetry, GetTracing()).WillOnce(::testing::Return(tracing_));
    server_filter_ = std::make_shared<OpenTelemetryTracingServerFilter>();
    ASSERT_EQ(0, server_filter_->Init());

    trpc_service_ = std::make_shared<RpcServiceImpl>();
    ServiceAdapterOption option;
    option.protocol = "trpc";
    adapter_ = std::make_unique<trpc::ServiceAdapter>(std::move(option));
    trpc_service_->SetAdapter(adapter_.get());
    trpc::opentelemetry::SetServerTraceAttrsFunc(
        [](const ServerContextPtr& context, const void* req, std::unordered_map<std::string, std::string>& attributes) {
          attributes["testkey"] = "testvalue";
        });
  }

  static void TearDownTestCase() { UnregisterPlugins(); }

  static ServerContextPtr GetTestTrpcServerContext() {
    DummyTrpcProtocol req_data;
    trpc::test::helloworld::HelloRequest hello_req;
    NoncontiguousBuffer req_bin_data;
    PackTrpcRequest(req_data, static_cast<void*>(&hello_req), req_bin_data);
    ServerContextPtr context = MakeTestServerContext("trpc", trpc_service_.get(), std::move(req_bin_data));
    context->SetFuncName("test_func");
    context->SetDyeingKey("dyeing");
    return context;
  }

  static ServerContextPtr GetTestHttpServerContext() {
    http::RequestPtr request = std::make_shared<http::Request>(1000, false);
    http_service_ = std::make_shared<HttpService>();
    ServerContextPtr context = MakeTestServerContext("http", http_service_.get(), std::move(request));
    return context;
  }

 protected:
  static MessageServerFilterPtr server_filter_;
  static OpenTelemetryTracingPtr tracing_;
  static std::shared_ptr<trpc::ServiceAdapter> adapter_;
  static std::shared_ptr<RpcServiceImpl> trpc_service_;
  static std::shared_ptr<HttpService> http_service_;
};

MessageServerFilterPtr TracingServerFilterTest::server_filter_;
OpenTelemetryTracingPtr TracingServerFilterTest::tracing_;
std::shared_ptr<trpc::ServiceAdapter> TracingServerFilterTest::adapter_;
std::shared_ptr<RpcServiceImpl> TracingServerFilterTest::trpc_service_;
std::shared_ptr<HttpService> TracingServerFilterTest::http_service_;

class TestCarrier : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override {
    return "";
  }

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override {}
};

trpc::opentelemetry::TextMapCarrierPtr ServerTestCarrierFunc(const trpc::ServerContextPtr& context) {
  return std::make_unique<TestCarrier>();
}

TEST_F(TracingServerFilterTest, ServerCarrier) {
  // for protocols that are not set, the default implementation will be returned
  ASSERT_NE(nullptr, trpc::opentelemetry::GetServerCarrierFunc("not_set_protocol"));

  // testing for setter and getter
  trpc::opentelemetry::SetServerCarrierFunc("test_protocol", ServerTestCarrierFunc);
  ASSERT_NE(nullptr, trpc::opentelemetry::GetServerCarrierFunc("test_protocol"));
  trpc::opentelemetry::SetServerCarrierFunc("test_protocol", nullptr);
  ASSERT_NE(nullptr, trpc::opentelemetry::GetServerCarrierFunc("test_protocol"));

  // testing for ServerTransInfoCarrierFunc
  ServerContextPtr trpc_context = GetTestTrpcServerContext();
  ASSERT_NE(nullptr, trpc::opentelemetry::ServerTransInfoCarrierFunc(trpc_context));

  // testing for ServerHttpCarrierFunc
  ServerContextPtr http_context = GetTestHttpServerContext();
  ASSERT_NE(nullptr, trpc::opentelemetry::ServerHttpCarrierFunc(http_context));
}

TEST_F(TracingServerFilterTest, Init) {
  ASSERT_EQ(trpc::opentelemetry::kOpenTelemetryTelemetryName, server_filter_->Name());
  std::vector<FilterPoint> points = server_filter_->GetFilterPoint();
  ASSERT_EQ(2, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_POST_RPC_INVOKE) != points.end());
}

TEST_F(TracingServerFilterTest, TraceWithoutUpstreamInfo) {
  FilterStatus status;

  // 1. tests for SERVER_PRE_RPC_INVOKE filter point
  // There is no tracing information in the context's transinfo.
  ServerContextPtr context = GetTestTrpcServerContext();
  server_filter_->operator()(status, FilterPoint::SERVER_PRE_RPC_INVOKE, context);
  ASSERT_EQ(status, FilterStatus::CONTINUE);
  ServerTracingSpan* ptr = context->GetFilterData<ServerTracingSpan>(tracing_->GetPluginID());
  ASSERT_NE(nullptr, ptr);
  ASSERT_EQ(typeid(trpc::opentelemetry::OpenTelemetryTracingSpanPtr), ptr->span.type());
  auto& span = std::any_cast<trpc::opentelemetry::OpenTelemetryTracingSpanPtr&>(ptr->span);
  ASSERT_NE(nullptr, span.get());
  // it is in the recording state before reporting
  ASSERT_TRUE(span->IsRecording());

  // 2. tests for SERVER_POST_RPC_INVOKE filter point
  trpc::Status frame_status;
  frame_status.SetFrameworkRetCode(101);
  context->SetStatus(frame_status);
  server_filter_->operator()(status, FilterPoint::SERVER_POST_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  // after reporting, it is in a non-recording state
  ASSERT_FALSE(span->IsRecording());
}

TEST_F(TracingServerFilterTest, TraceWithUpstreamInfo) {
  FilterStatus status;

  // 1. tests for SERVER_PRE_RPC_INVOKE filter point
  // There has tracing information in the context's transinfo.
  ServerContextPtr context = GetTestTrpcServerContext();
  std::string parent_trace_id = "10000000000000000000000000000000";
  std::string parent_span_id = "2000000000000000";
  std::string trace_info_str = "00-" + parent_trace_id + "-" + parent_span_id + "-01";
  context->AddReqTransInfo(std::string(::opentelemetry::trace::propagation::kTraceParent), trace_info_str);
  server_filter_->operator()(status, FilterPoint::SERVER_PRE_RPC_INVOKE, context);
  ASSERT_EQ(status, FilterStatus::CONTINUE);
  ServerTracingSpan* ptr = context->GetFilterData<ServerTracingSpan>(tracing_->GetPluginID());
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
  ASSERT_EQ(parent_trace_id, std::string(trace_id, sizeof(trace_id)));
  char span_id[16];
  span->GetContext().span_id().ToLowerBase16(span_id);
  ASSERT_NE(parent_span_id, std::string(span_id, sizeof(span_id)));

  // 2. tests for SERVER_POST_RPC_INVOKE filter point
  trpc::Status frame_status;
  frame_status.SetFrameworkRetCode(101);
  context->SetStatus(frame_status);
  server_filter_->operator()(status, FilterPoint::SERVER_POST_RPC_INVOKE, context);
  ASSERT_EQ(FilterStatus::CONTINUE, status);
  // after reporting, it is in a non-recording state
  ASSERT_FALSE(span->IsRecording());
}

}  // namespace trpc::testing
