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

#ifdef ENABLE_LOGS_PREVIEW
#include "trpc/telemetry/opentelemetry/logging/grpc_log_exporter.h"

#include "gtest/gtest.h"
#include "trpc/client/testing/service_proxy_testing.h"
#include "trpc/client/trpc_client.h"

#include "trpc/telemetry/opentelemetry/logging/common.h"
#include "trpc/telemetry/opentelemetry/logging/logs_service.trpc.pb.mock.h"

namespace trpc::testing {

class GrpcLogExporterTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() { RegisterPlugins(); }
  static void TearDownTestCase() {
    trpc::GetTrpcClient()->Stop();
    UnregisterPlugins();
    trpc::GetTrpcClient()->Destroy();
  }
};

TEST_F(GrpcLogExporterTest, Export) {
  ServiceProxyOption options;
  options.name = trpc::opentelemetry::kGrpcLogExporterServiceName;
  options.codec_name = "grpc";
  options.selector_name = "direct";
  options.target = "127.0.0.1:8888";
  options.threadmodel_type_name = kSeparate;
  options.threadmodel_instance_name = kSeparateAdminInstance;
  auto mock_proxy = GetTrpcClient()->GetProxy<::opentelemetry::proto::collector::logs::v1::MockLogsServiceServiceProxy>(
      options.name, &options);
  auto exporter = std::make_shared<trpc::opentelemetry::GrpcLogExporter>(mock_proxy);

  // 1. exports empty records
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::logs::Recordable>> empty_records;
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kSuccess, exporter->Export(empty_records));

  // 2. exports success
  auto succ_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::logs::Recordable>> succ_records(&succ_recordable,
                                                                                                     1);
  EXPECT_CALL(*mock_proxy, Export(::testing::_, ::testing::_, ::testing::_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(trpc::kSuccStatus));
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kSuccess, exporter->Export(succ_records));

  // 3. exports failed
  auto fail_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::logs::Recordable>> fail_records(&fail_recordable,
                                                                                                     1);
  EXPECT_CALL(*mock_proxy, Export(::testing::_, ::testing::_, ::testing::_))
      .Times(::testing::Exactly(1))
      .WillOnce(::testing::Return(trpc::Status(-1, "")));
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kFailure, exporter->Export(fail_records));

  // 4. exports after shutdown
  exporter->Shutdown();
  auto shutdown_recordable = exporter->MakeRecordable();
  ::opentelemetry::nostd::span<std::unique_ptr<::opentelemetry::sdk::logs::Recordable>> shutdown_records(
      &shutdown_recordable, 1);
  ASSERT_EQ(::opentelemetry::sdk::common::ExportResult::kFailure, exporter->Export(shutdown_records));
}

}  // namespace trpc::testing
#endif
