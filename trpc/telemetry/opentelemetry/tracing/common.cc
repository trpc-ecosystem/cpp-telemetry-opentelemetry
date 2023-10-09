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

#include "trpc/util/log/logging.h"
#include "trpc/util/pb2json.h"

namespace trpc::opentelemetry {

// The max length of the request/response data that allowed to be reported
uint32_t max_string_length = kDefaultMaxStringLength;

// The span's startup attributes setting function of server side
ServerTraceAttributesFunc server_trace_attrs_func = nullptr;

// The span's startup attributes setting function of client side
ClientTraceAttributesFunc client_trace_attrs_func = nullptr;

uint32_t GetMaxStringLength() { return max_string_length; }

void SetMaxStringLength(uint32_t limit) {
  if (limit < strlen(kFixedStringSuffix)) {
    TRPC_FMT_ERROR("max_string_length can not be set too small! It remains {}", max_string_length);
    return;
  }
  max_string_length = limit;
}

ServerTraceAttributesFunc GetServerTraceAttrsFunc() { return server_trace_attrs_func; }

ClientTraceAttributesFunc GetClientTraceAttrsFunc() { return client_trace_attrs_func; }

void SetServerTraceAttrsFunc(ServerTraceAttributesFunc func) { server_trace_attrs_func = func; }

void SetClientTraceAttrsFunc(ClientTraceAttributesFunc func) { client_trace_attrs_func = func; }

namespace detail {

void GetMsgJsonData(const google::protobuf::Message* pb_msg, std::string& json_data) {
  Pb2Json::PbToJson(*pb_msg, &json_data);
  if (json_data.length() > GetMaxStringLength()) {
    json_data = json_data.substr(0, GetMaxStringLength() - strlen(kFixedStringSuffix)) + kFixedStringSuffix;
  }
}

}  // namespace detail

}  // namespace trpc::opentelemetry
