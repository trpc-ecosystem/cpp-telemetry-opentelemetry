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

#include "gtest/gtest.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/codec/trpc/testing/trpc_protocol_testing.h"
#include "trpc/common/config/trpc_config.h"
#include "trpc/proto/testing/helloworld.pb.h"
#include "trpc/server/rpc/rpc_service_impl.h"
#include "trpc/server/service_adapter.h"
#include "trpc/server/testing/server_context_testing.h"
#include "trpc/telemetry/telemetry_factory.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry.h"

namespace trpc::testing {

class OpenTelemetryServerFilterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    int ret =
        TrpcConfig::GetInstance()->Init("./trpc/telemetry/opentelemetry/testing/opentelemetry_telemetry_test.yaml");
    ASSERT_EQ(0, ret);
    RegisterPlugins();
    ASSERT_TRUE(log::Init());

    // registers OpenTelemetryTelemetry
    TelemetryPtr telemetry = MakeRefCounted<OpenTelemetryTelemetry>();
    TelemetryFactory::GetInstance()->Register(telemetry);
    ASSERT_EQ(0, telemetry->Init());

    // initializes filter
    filter_ = std::make_shared<OpenTelemetryServerFilter>();
    ASSERT_EQ(0, filter_->Init());
  }

  static void TearDownTestCase() {
    log::Destroy();
    UnregisterPlugins();
  }

 protected:
  static std::shared_ptr<OpenTelemetryServerFilter> filter_;
};

std::shared_ptr<OpenTelemetryServerFilter> OpenTelemetryServerFilterTest::filter_;

TEST_F(OpenTelemetryServerFilterTest, GetFilterPoint) {
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  auto points = filter_->GetFilterPoint();
  ASSERT_EQ(4, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_POST_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_POST_RECV_MSG) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_PRE_SEND_MSG) != points.end());
#else
  auto points = filter_->GetFilterPoint();
  ASSERT_EQ(2, points.size());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_PRE_RPC_INVOKE) != points.end());
  ASSERT_TRUE(std::find(points.begin(), points.end(), FilterPoint::SERVER_POST_RPC_INVOKE) != points.end());
#endif
}

TEST_F(OpenTelemetryServerFilterTest, DoFilter) {
  DummyTrpcProtocol req_data;
  trpc::test::helloworld::HelloRequest hello_req;
  NoncontiguousBuffer req_bin_data;
  PackTrpcRequest(req_data, static_cast<void*>(&hello_req), req_bin_data);
  auto trpc_service = std::make_shared<RpcServiceImpl>();
  ServiceAdapterOption option;
  option.protocol = "trpc";
  std::shared_ptr<trpc::ServiceAdapter> adapter = std::make_unique<trpc::ServiceAdapter>(std::move(option));
  trpc_service->SetAdapter(adapter.get());
  ServerContextPtr context = MakeTestServerContext("trpc", trpc_service.get(), std::move(req_bin_data));

  FilterStatus status;
  (*filter_)(status, FilterPoint::SERVER_PRE_RPC_INVOKE, context);
  (*filter_)(status, FilterPoint::SERVER_POST_RPC_INVOKE, context);
#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
  (*filter_)(status, FilterPoint::SERVER_POST_RECV_MSG, context);
  (*filter_)(status, FilterPoint::SERVER_PRE_SEND_MSG, context);
#endif
}

}  // namespace trpc::testing
