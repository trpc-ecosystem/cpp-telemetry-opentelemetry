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

#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "google/protobuf/map.h"
#include "opentelemetry/context/propagation/text_map_propagator.h"
#include "trpc/util/http/request.h"

namespace trpc::opentelemetry {

using TextMapCarrierPtr = std::unique_ptr<::opentelemetry::context::propagation::TextMapCarrier>;

/// @brief A read-write class that assists in setting tracing information to transinfo.
class TransInfoWriter : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  explicit TransInfoWriter(google::protobuf::Map<std::string, std::string>* text_map) : text_map_(text_map) {}

  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override;

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override;

 private:
  google::protobuf::Map<std::string, std::string>* text_map_;
};

/// @brief A read-only class that assists in extracting tracing information from transinfo.
class TransInfoReader : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  explicit TransInfoReader(const google::protobuf::Map<std::string, std::string>& text_map) : text_map_(text_map) {}

  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override;

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override {}

 private:
  const google::protobuf::Map<std::string, std::string>& text_map_;
};

/// @brief A read-write class that assists in setting tracing information to Http header.
class HttpHeaderWriter : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  explicit HttpHeaderWriter(http::Request* http_request) : http_request_(http_request) {}

  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override;

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override;

 private:
  http::Request* http_request_;
};

/// @brief A read-only class that assists in extracting tracing information from Http header.
class HttpHeaderReader : public ::opentelemetry::context::propagation::TextMapCarrier {
 public:
  explicit HttpHeaderReader(const http::Request& http_request) : http_request_(http_request) {}

  ::opentelemetry::nostd::string_view Get(::opentelemetry::nostd::string_view key) const noexcept override;

  void Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept override {}

 private:
  const http::Request& http_request_;

  // stores the trace information obtained from the "trpc-trans-info" field in the Http header.
  mutable std::unordered_map<std::string, std::string> trace_info_;
};

}  // namespace trpc::opentelemetry
