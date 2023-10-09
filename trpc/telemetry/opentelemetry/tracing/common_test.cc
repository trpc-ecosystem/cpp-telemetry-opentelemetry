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

#include "trpc/telemetry/opentelemetry/tracing/common.h"

#include "gtest/gtest.h"

#include "trpc/proto/testing/helloworld.pb.h"

namespace trpc::testing {

TEST(OpenTelemetryTracingCommonTest, MaxStringLength) {
  ASSERT_EQ(trpc::opentelemetry::kDefaultMaxStringLength, trpc::opentelemetry::GetMaxStringLength());

  uint32_t small_value = 0;
  trpc::opentelemetry::SetMaxStringLength(small_value);
  ASSERT_NE(small_value, trpc::opentelemetry::GetMaxStringLength());
  ASSERT_EQ(trpc::opentelemetry::kDefaultMaxStringLength, trpc::opentelemetry::GetMaxStringLength());

  uint32_t normal_value = strlen(trpc::opentelemetry::kFixedStringSuffix) + 1;
  trpc::opentelemetry::SetMaxStringLength(normal_value);
  ASSERT_EQ(normal_value, trpc::opentelemetry::GetMaxStringLength());
  ASSERT_NE(trpc::opentelemetry::kDefaultMaxStringLength, trpc::opentelemetry::GetMaxStringLength());
}

TEST(OpenTelemetryTracingCommonTest, ServerTraceAttrsFunc) {
  ASSERT_EQ(nullptr, trpc::opentelemetry::GetServerTraceAttrsFunc());

  trpc::opentelemetry::SetServerTraceAttrsFunc(
      [](const ServerContextPtr& context, const void* req, std::unordered_map<std::string, std::string>& attributes) {
        attributes["testkey"] = "testvalue";
      });
  ASSERT_NE(nullptr, trpc::opentelemetry::GetServerTraceAttrsFunc());
}

TEST(OpenTelemetryTracingCommonTest, ClientTraceAttrsFunc) {
  ASSERT_EQ(nullptr, trpc::opentelemetry::GetClientTraceAttrsFunc());

  trpc::opentelemetry::SetClientTraceAttrsFunc(
      [](const ClientContextPtr& context, const void* req, std::unordered_map<std::string, std::string>& attributes) {
        attributes["testkey"] = "testvalue";
      });
  ASSERT_NE(nullptr, trpc::opentelemetry::GetClientTraceAttrsFunc());
}

TEST(OpenTelemetryTracingCommonTest, GetMsgJsonData) {
  trpc::test::helloworld::HelloRequest hello_req;
  trpc::opentelemetry::SetMaxStringLength(100);

  // test for getting without truncation
  std::string short_str(10, 'a');
  hello_req.set_msg(short_str);
  std::string short_report_data;
  trpc::opentelemetry::detail::GetMsgJsonData(&hello_req, short_report_data);
  ASSERT_LT(short_report_data.length(), trpc::opentelemetry::GetMaxStringLength());

  // test for getting with truncation
  std::string long_str(trpc::opentelemetry::GetMaxStringLength() + 1, 'a');
  hello_req.set_msg(long_str);
  std::string long_report_data;
  trpc::opentelemetry::detail::GetMsgJsonData(&hello_req, long_report_data);
  ASSERT_EQ(long_report_data.length(), trpc::opentelemetry::GetMaxStringLength());
}

}  // namespace trpc::testing
