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

#include "trpc/telemetry/opentelemetry/metrics/common.h"

#include "gtest/gtest.h"
#include "trpc/codec/codec_helper.h"

namespace trpc::testing {

class OpenTelemetryMetricsCommonTest : public ::testing::Test {
 protected:
  static void SetUpTestCase() {
    std::vector<trpc::OpenTelemetryMetricsCode> codes;
    codes.push_back(trpc::OpenTelemetryMetricsCode{
        .code = 10001, .type = trpc::opentelemetry::kSuccessType, .description = "user success"});
    codes.push_back(trpc::OpenTelemetryMetricsCode{.code = 10002,
                                                   .type = trpc::opentelemetry::kTimeoutType,
                                                   .description = "user timeout",
                                                   .service = "service1"});
    codes.push_back(trpc::OpenTelemetryMetricsCode{
        .code = 10003, .type = trpc::opentelemetry::kExceptionType, .description = "user exception"});
    codes.push_back(trpc::OpenTelemetryMetricsCode{.code = 10004, .type = "other", .description = "other"});
    trpc::opentelemetry::InitUserCodeMap(codes);

    trpc::opentelemetry::InitDefaultCodeMap();
  }
  static void TearDownTestCase() {}
};

TEST_F(OpenTelemetryMetricsCommonTest, UserDefine) {
  // match success configuration
  std::map<std::string, std::string> module_infos;
  trpc::opentelemetry::SetCallResult(10001, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kSuccessType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("user success", module_infos[trpc::opentelemetry::kCodeDesc]);

  // match timeout configuration
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(10002, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kTimeoutType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("user timeout", module_infos[trpc::opentelemetry::kCodeDesc]);

  // service does not match
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(10002, "service2", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_NE(trpc::opentelemetry::kTimeoutType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ(trpc::opentelemetry::kExceptionType, module_infos[trpc::opentelemetry::kCodeType]);

  // match exception configuration
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(10003, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kExceptionType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("user exception", module_infos[trpc::opentelemetry::kCodeDesc]);

  // "other" is not a valid option and it is treated as exception
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(10004, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_NE("other", module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ(trpc::opentelemetry::kExceptionType, module_infos[trpc::opentelemetry::kCodeType]);
}

TEST_F(OpenTelemetryMetricsCommonTest, DefaultDefine) {
  // success
  std::map<std::string, std::string> module_infos;
  trpc::opentelemetry::SetCallResult(trpc::TrpcRetCode::TRPC_INVOKE_SUCCESS, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kSuccessType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("code=0", module_infos[trpc::opentelemetry::kCodeDesc]);

  // client timeout
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(trpc::TrpcRetCode::TRPC_CLIENT_INVOKE_TIMEOUT_ERR, "service1", "method1",
                                     module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kTimeoutType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("client timeout", module_infos[trpc::opentelemetry::kCodeDesc]);

  // server timeout
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(trpc::TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kTimeoutType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("server timeout", module_infos[trpc::opentelemetry::kCodeDesc]);

  // other error codes are treated as exceptions
  module_infos.clear();
  trpc::opentelemetry::SetCallResult(30000, "service1", "method1", module_infos);
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeType));
  ASSERT_NE(module_infos.end(), module_infos.find(trpc::opentelemetry::kCodeDesc));
  ASSERT_EQ(trpc::opentelemetry::kExceptionType, module_infos[trpc::opentelemetry::kCodeType]);
  ASSERT_EQ("code!=0", module_infos[trpc::opentelemetry::kCodeDesc]);
}

}  // namespace trpc::testing
