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

#include "rapidjson/document.h"
#include "trpc/util/log/logging.h"

namespace trpc::opentelemetry {

::opentelemetry::nostd::string_view TransInfoWriter::Get(::opentelemetry::nostd::string_view key) const noexcept {
  if (!text_map_) {
    TRPC_LOG_TRACE("get fail! text_map is null");
    return "";
  }
  auto iter = (*text_map_).find(std::string(key));
  if (iter != (*text_map_).end()) {
    return ::opentelemetry::nostd::string_view(iter->second);
  } else {
    return "";
  }
}

void TransInfoWriter::Set(::opentelemetry::nostd::string_view key, ::opentelemetry::nostd::string_view value) noexcept {
  if (!text_map_) {
    TRPC_LOG_TRACE("set fail! text_map is null");
    return;
  }
  (*text_map_)[std::string(key)] = std::string(value);
}

::opentelemetry::nostd::string_view TransInfoReader::Get(::opentelemetry::nostd::string_view key) const noexcept {
  auto iter = text_map_.find(std::string(key));
  if (iter != text_map_.end()) {
    return ::opentelemetry::nostd::string_view(iter->second);
  } else {
    return "";
  }
}

::opentelemetry::nostd::string_view HttpHeaderWriter::Get(::opentelemetry::nostd::string_view key) const noexcept {
  if (!http_request_) {
    TRPC_LOG_TRACE("get fail! http_request is null");
    return "";
  }
  std::string key_str(key);
  return ::opentelemetry::nostd::string_view(http_request_->GetHeader(key_str));
}

void HttpHeaderWriter::Set(::opentelemetry::nostd::string_view key,
                           ::opentelemetry::nostd::string_view value) noexcept {
  if (!http_request_) {
    TRPC_LOG_TRACE("set fail! http_request is null");
    return;
  }
  http_request_->SetHeader(std::string(key), std::string(value));
}

::opentelemetry::nostd::string_view HttpHeaderReader::Get(::opentelemetry::nostd::string_view key) const noexcept {
  // search for the key in the Http header directly.
  std::string key_str(key);
  if (http_request_.HasHeader(key_str)) {
    return ::opentelemetry::nostd::string_view(http_request_.GetHeader(key_str));
  }

  // search for the key in the "trpc-trans-info" field of the Http header.
  // the "trpc-trans-info" field is in json format.
  if (http_request_.HasHeader("trpc-trans-info")) {
    rapidjson::Document document;
    rapidjson::ParseResult parse_ok = document.Parse(http_request_.GetHeader("trpc-trans-info").c_str());
    if (parse_ok) {
      if (document.IsObject() && document.HasMember(key_str.c_str()) && document[key_str.c_str()].IsString()) {
        std::string value = document[key_str.c_str()].GetString();
        // Since the Get interface returns a string_view, it need to store the value instead of returning a temporary
        // variable directly.
        trace_info_[key_str] = std::move(value);
        return ::opentelemetry::nostd::string_view(trace_info_[key_str]);
      }
    }
  }

  return "";
}

}  // namespace trpc::opentelemetry
