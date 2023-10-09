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

#include "trpc/telemetry/opentelemetry/tracing/grpc_trace_exporter.h"

#include "gtest/gtest.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/client/trpc_client.h"

#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/telemetry/opentelemetry/tracing/trace_service.trpc.pb.mock.h"

namespace trpc::testing {

class GrpcTraceExporterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { RegisterPlugins(); }
  static void TearDownTestCase() {
    trpc::GetTrpcClient()->Stop();
    UnregisterPlugins();
    trpc::GetTrpcClient()->Destroy();
  }
};

TEST_F(GrpcTraceExporterTest, Export) {
  ServiceProxyOption options;
  options.codec_name = "grpc";
  options.selector_name = "direct";
  options.target = "127.0.0.1:8888";
  options.threadmodel_type_name = kSeparate;
  options.threadmodel_instance_name = kSeparateAdminInstance;
  auto mock_proxy =
      trpc::GetTrpcClient()->GetProxy<::opentelemetry::proto::collector::trace::v1::MockTraceServiceServiceProxy>(
          trpc::opentelemetry::kGrpcTraceExporterServiceName, &options);
  auto exporter = std::make_shared<trpc::opentelemetry::GrpcTraceExporter>(mock_proxy);

  // 1. exports empty spans
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>> empty_spans;
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kSuccess, exporter->Export(empty_spans));

  // 2. exports success
  auto succ_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>> succ_spans(&succ_recordable,
                                                                                                    1);
  EXPECT_CALL(*mock_proxy, Export(::testing::_, ::testing::_, ::testing::_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(::trpc::kSuccStatus));
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kSuccess, exporter->Export(succ_spans));

  // 3. exports failed
  auto fail_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>> fail_spans(&fail_recordable,
                                                                                                    1);
  EXPECT_CALL(*mock_proxy, Export(::testing::_, ::testing::_, ::testing::_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(::trpc::Status(-1, "")));
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kFailure, exporter->Export(fail_spans));

  // 4. exports after shutdown
  exporter->Shutdown();
  auto shutdown_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::trace::Recordable>> shutdown_spans(
      &shutdown_recordable, 1);
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kFailure, exporter->Export(shutdown_spans));
}

}  // namespace trpc::testing
