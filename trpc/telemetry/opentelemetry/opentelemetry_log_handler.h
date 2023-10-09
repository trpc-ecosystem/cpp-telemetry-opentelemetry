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

#include "opentelemetry/sdk/common/global_log_handler.h"

namespace trpc::opentelemetry {

/// @brief Log processing handler for the OpenTelemetry library. LogHandler::Handle will be called when logging macros
///        are used in the OpenTelemetry library.
class LogHandler : public ::opentelemetry::sdk::common::internal_log::LogHandler {
 public:
  void Handle(::opentelemetry::sdk::common::internal_log::LogLevel level, const char* file, int line, const char* msg,
              const ::opentelemetry::sdk::common::AttributeMap& attributes) noexcept override;
};

}  // namespace trpc::opentelemetry
