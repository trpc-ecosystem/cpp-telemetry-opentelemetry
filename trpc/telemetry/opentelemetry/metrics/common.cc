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

#include <set>

#include "trpc/codec/codec_helper.h"
#include "trpc/util/log/logging.h"

namespace trpc::opentelemetry {

namespace {

/// @brief The map for user-configured error code mapping
std::unordered_map<int, std::vector<OpenTelemetryMetricsCode> > user_code_map;

/// @brief The map for default error code mapping
std::unordered_map<int, OpenTelemetryMetricsCode> default_code_map;

bool IsServiceMatch(const OpenTelemetryMetricsCode& metric_code, const std::string& service_name) {
  if (metric_code.service == "" || metric_code.service == service_name) {
    return true;
  }
  return false;
}

bool IsMethodMatch(const OpenTelemetryMetricsCode& metric_code, const std::string& method) {
  if (metric_code.method == "" || metric_code.method == method) {
    return true;
  }
  return false;
}

bool IsMatchUserDefine(const OpenTelemetryMetricsCode& metric_code, const std::string& service_name,
                       const std::string& method) {
  if (IsServiceMatch(metric_code, service_name) && IsMethodMatch(metric_code, method)) {
    return true;
  }
  return false;
}

void SetCallResultByDefault(int ret_code, std::map<std::string, std::string>& module_infos) {
  if (default_code_map.find(ret_code) != default_code_map.end()) {
    module_infos[kCodeType] = default_code_map[ret_code].type;
    module_infos[kCodeDesc] = default_code_map[ret_code].description;
    return;
  }
  // all other cases should be treated as exceptions
  module_infos[kCodeType] = trpc::opentelemetry::kExceptionType;
  module_infos[kCodeDesc] = "code!=0";
}

}  // namespace

void InitUserCodeMap(const std::vector<OpenTelemetryMetricsCode>& codes) {
  // "type" can only be one of three options:
  std::set<std::string> type_set = {kSuccessType, kTimeoutType, kExceptionType};

  for (auto metric_code : codes) {
    if (type_set.count(metric_code.type) == 0) {
      TRPC_LOG_ERROR("code type " << metric_code.type << " is not support");
      continue;
    }
    user_code_map[metric_code.code].push_back(metric_code);
  }
}

void InitDefaultCodeMap() {
  default_code_map[trpc::TrpcRetCode::TRPC_INVOKE_SUCCESS] = OpenTelemetryMetricsCode{
      .code = trpc::TrpcRetCode::TRPC_INVOKE_SUCCESS, .type = kSuccessType, .description = "code=0"};
  default_code_map[trpc::TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR] = OpenTelemetryMetricsCode{
      .code = trpc::TrpcRetCode::TRPC_SERVER_TIMEOUT_ERR, .type = kTimeoutType, .description = "server timeout"};
  default_code_map[trpc::TrpcRetCode::TRPC_SERVER_FULL_LINK_TIMEOUT_ERR] =
      OpenTelemetryMetricsCode{.code = trpc::TrpcRetCode::TRPC_SERVER_FULL_LINK_TIMEOUT_ERR,
                               .type = kTimeoutType,
                               .description = "server fulllink timeout"};
  default_code_map[trpc::TrpcRetCode::TRPC_CLIENT_INVOKE_TIMEOUT_ERR] = OpenTelemetryMetricsCode{
      .code = trpc::TrpcRetCode::TRPC_CLIENT_INVOKE_TIMEOUT_ERR, .type = kTimeoutType, .description = "client timeout"};
  default_code_map[trpc::TrpcRetCode::TRPC_CLIENT_FULL_LINK_TIMEOUT_ERR] =
      OpenTelemetryMetricsCode{.code = trpc::TrpcRetCode::TRPC_CLIENT_FULL_LINK_TIMEOUT_ERR,
                               .type = kTimeoutType,
                               .description = "client fulllink timeout"};
}

void SetCallResult(int ret_code, const std::string& service_name, const std::string& method,
                   std::map<std::string, std::string>& module_infos) {
  module_infos[kCode] = std::to_string(ret_code);
  if (user_code_map.find(ret_code) != user_code_map.end()) {
    for (auto& metric_code : user_code_map[ret_code]) {
      if (IsMatchUserDefine(metric_code, service_name, method)) {
        module_infos[kCodeType] = metric_code.type;
        module_infos[kCodeDesc] = metric_code.description;
        return;
      }
    }
  }
  SetCallResultByDefault(ret_code, module_infos);
}

}  // namespace trpc::opentelemetry
