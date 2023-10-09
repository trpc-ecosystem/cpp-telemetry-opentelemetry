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

#include <string>
#include <vector>

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_conf.h"

namespace trpc::opentelemetry {

/// @brief Types of module metrics reported by the framework.
enum class ModuleReportType {
  kClientStartedCount = 0,
  kClientHandledCount = 1,
  kClientHandledTime = 2,
  kServerStartedCount = 3,
  kServerHandledCount = 4,
  kServerHandledTime = 5,
};

/// @brief The key definition reported by the filter
constexpr char kCallerService[] = "caller_service";
constexpr char kCallerMethod[] = "caller_method";
constexpr char kCalleeService[] = "callee_service";
constexpr char kCalleeMethod[] = "callee_method";
constexpr char kCode[] = "code";
constexpr char kCodeType[] = "code_type";
constexpr char kCodeDesc[] = "code_desc";

/// @brief Types of code
constexpr char kSuccessType[] = "success";
constexpr char kTimeoutType[] = "timeout";
constexpr char kExceptionType[] = "exception";

/// @brief Initializes the user-configured error code mapping map.
/// @param codes the user-configured error code mapping information
void InitUserCodeMap(const std::vector<OpenTelemetryMetricsCode>& codes);

/// @brief Initializes the default error code mapping map.
void InitDefaultCodeMap();

/// @brief Sets the call result into the metrics info based on the error code.
/// @param ret_code error code
/// @param service_name servive name
/// @param method method
/// @param [out] metrics_info metrics data for inter-module calls
void SetCallResult(int ret_code, const std::string& service_name, const std::string& method,
                   std::map<std::string, std::string>& module_infos);

}  // namespace trpc::opentelemetry
