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

#include "trpc/telemetry/opentelemetry/tracing/text_map_carrier.h"

#include "gtest/gtest.h"

namespace trpc::testing {

TEST(TransInfoWriterTest, Get) {
  google::protobuf::Map<std::string, std::string> text_map;
  text_map["testkey"] = "testvalue";
  trpc::opentelemetry::TransInfoWriter carrier(&text_map);
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));

  trpc::opentelemetry::TransInfoWriter null_carrier(nullptr);
  ASSERT_EQ("", std::string(null_carrier.Get("testkey")));
}

TEST(TransInfoWriterTest, Set) {
  google::protobuf::Map<std::string, std::string> text_map;
  trpc::opentelemetry::TransInfoWriter carrier(&text_map);
  carrier.Set("testkey", "testvalue");
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));

  trpc::opentelemetry::TransInfoWriter null_carrier(nullptr);
  null_carrier.Set("testkey", "testvalue");
  ASSERT_EQ("", std::string(null_carrier.Get("testkey")));
}

TEST(TransInfoReaderTest, Get) {
  google::protobuf::Map<std::string, std::string> text_map;
  text_map["testkey"] = "testvalue";
  trpc::opentelemetry::TransInfoReader carrier(text_map);
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));
}

TEST(TransInfoReaderTest, Set) {
  google::protobuf::Map<std::string, std::string> text_map;
  trpc::opentelemetry::TransInfoReader carrier(text_map);
  carrier.Set("testkey", "testvalue");
  ASSERT_EQ("", std::string(carrier.Get("testkey")));
}

TEST(HttpHeaderWriterTest, Get) {
  http::Request http_request;
  http_request.SetHeader("testkey", "testvalue");
  trpc::opentelemetry::HttpHeaderWriter carrier(&http_request);
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));

  trpc::opentelemetry::HttpHeaderWriter null_carrier(nullptr);
  ASSERT_EQ("", std::string(null_carrier.Get("testkey")));
}

TEST(HttpHeaderWriterTest, Set) {
  http::Request http_request;
  trpc::opentelemetry::HttpHeaderWriter carrier(&http_request);
  carrier.Set("testkey", "testvalue");
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));

  trpc::opentelemetry::HttpHeaderWriter null_carrier(nullptr);
  null_carrier.Set("testkey", "testvalue");
  ASSERT_EQ("", std::string(null_carrier.Get("testkey")));
}

TEST(HttpHeaderReaderTest, Get) {
  // 1. test getting the value corresponding to the key directly from the Http header.
  http::Request http_request;
  http_request.SetHeader("testkey", "testvalue");
  trpc::opentelemetry::HttpHeaderReader carrier(http_request);
  ASSERT_EQ("testvalue", std::string(carrier.Get("testkey")));
  ASSERT_EQ("", std::string(carrier.Get("invalidkey")));

  // 2. test getting the value corresponding to the key form "trpc-trans-info" field of the Http header.
  http::Request tran_http_request;
  trpc::opentelemetry::HttpHeaderReader tran_carrier(tran_http_request);

  // 2.1 do not have "trpc-trans-info" field
  ASSERT_EQ("", std::string(tran_carrier.Get("testkey")));

  // 2.2 the format of "trpc-trans-info" is incorrect
  tran_http_request.SetHeader("trpc-trans-info", "{\"testkey\":: \"testvalue\"}");
  ASSERT_EQ("", std::string(tran_carrier.Get("testkey")));

  // 2.3 the "trpc-trans-info" field does not contain "testkey"
  tran_http_request.SetHeader("trpc-trans-info", "{\"testkeys\": \"testvalue\"}");
  ASSERT_EQ("", std::string(tran_carrier.Get("testkey")));

  // 2.4 the "trpc-trans-info" field contains "testkey"
  tran_http_request.SetHeader("trpc-trans-info", "{\"testkey\": \"testvalue\"}");
  ASSERT_EQ("testvalue", std::string(tran_carrier.Get("testkey")));
}

TEST(HttpHeaderReaderTest, Set) {
  http::Request http_request;
  trpc::opentelemetry::HttpHeaderReader carrier(http_request);
  carrier.Set("testkey", "testvalue");
  ASSERT_EQ("", std::string(carrier.Get("testkey")));
}

}  // namespace trpc::testing
