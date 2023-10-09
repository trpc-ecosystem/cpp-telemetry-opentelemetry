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

#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing_api.h"

#include "gtest/gtest.h"
#include "opentelemetry/trace/tracer.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/codec/trpc/testing/trpc_protocol_testing.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/proto/testing/helloworld.pb.h"
#include "trpc/server/rpc/rpc_service_impl.h"
#include "trpc/server/testing/server_context_testing.h"
#include "trpc/telemetry/telemetry_factory.h"
#include "trpc/tracing/tracing_filter_index.h"

#include "trpc/telemetry/opentelemetry/testing/mock_telemetry.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing.h"

namespace trpc::testing {

class OpenTelemetryTracingAPITest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);
    RegisterPlugins();

    trpc_service_ = std::make_shared<RpcServiceImpl>();

    // registers a telemetry plugin to enable successful invocation of tracing api.
    telemetry_ = MakeRefCounted<MockOpenTelemetryTelemetry>();
    tracing_ = MakeRefCounted<OpenTelemetryTracing>();
    ASSERT_EQ(0, tracing_->Init());
    TelemetryFactory::GetInstance()->Register(telemetry_);
    EXPECT_CALL(*telemetry_, GetTracing()).WillRepeatedly(::testing::Return(tracing_));
  }

  static void TearDownTestCase() { UnregisterPlugins(); }

  static ServerContextPtr GetTestServerContext(std::string& trace_id, std::string& span_id) {
    DummyTrpcProtocol req_data;
    trpc::test::helloworld::HelloRequest hello_req;
    NoncontiguousBuffer req_bin_data;
    PackTrpcRequest(req_data, static_cast<void*>(&hello_req), req_bin_data);
    ServerContextPtr context = MakeTestServerContext("trpc", trpc_service_.get(), std::move(req_bin_data));

    // sets span
    std::string err_msg;
    auto tracer = tracing_->MakeTracer(context->GetCallerName().c_str(), err_msg);
    // creates a parent span and save its trace id and span id
    auto span = tracer->StartSpan(context->GetFuncName(), {}, ::opentelemetry::trace::StartSpanOptions{});
    char trace_id_buf[32];
    span->GetContext().trace_id().ToLowerBase16(trace_id_buf);
    trace_id = std::string(trace_id_buf, sizeof(trace_id_buf));
    char span_id_buf[16];
    span->GetContext().span_id().ToLowerBase16(span_id_buf);
    span_id = std::string(span_id_buf, sizeof(span_id_buf));
    ServerTracingSpan server_span;
    server_span.span = std::move(trpc::opentelemetry::OpenTelemetryTracingSpanPtr(std::move(span)));
    context->SetFilterData(tracing_->GetPluginID(), std::move(server_span));

    return context;
  }

 protected:
  static OpenTelemetryTracingPtr tracing_;
  static std::shared_ptr<RpcServiceImpl> trpc_service_;
  static RefPtr<MockOpenTelemetryTelemetry> telemetry_;
};

OpenTelemetryTracingPtr OpenTelemetryTracingAPITest::tracing_;
std::shared_ptr<RpcServiceImpl> OpenTelemetryTracingAPITest::trpc_service_;
RefPtr<MockOpenTelemetryTelemetry> OpenTelemetryTracingAPITest::telemetry_;

TEST_F(OpenTelemetryTracingAPITest, GetTracingFilterDataIndex) {
  uint32_t index = trpc::opentelemetry::GetTracingFilterDataIndex();
  ASSERT_NE(trpc::opentelemetry::kInvalidTracingFilterDataIndex, index);
  ASSERT_EQ(tracing_->GetPluginID(), index);
}

TEST_F(OpenTelemetryTracingAPITest, GetTracingInfo) {
  std::string trace_id, span_id;
  ServerContextPtr context = GetTestServerContext(trace_id, span_id);
  ASSERT_NE(nullptr, trpc::opentelemetry::GetTracingSpan(context).get());
  ASSERT_EQ(trace_id, trpc::opentelemetry::GetTraceID(context));
  ASSERT_EQ(span_id, trpc::opentelemetry::GetSpanID(context));
}

}  // namespace trpc::testing
