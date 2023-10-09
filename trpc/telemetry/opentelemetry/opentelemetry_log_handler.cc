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

#include "trpc//telemetry/opentelemetry/opentelemetry_log_handler.h"

#include "trpc/util/log/logging.h"

namespace trpc::opentelemetry {

void LogHandler::Handle(::opentelemetry::sdk::common::internal_log::LogLevel level, const char* file, int line,
                        const char* msg, const ::opentelemetry::sdk::common::AttributeMap& attributes) noexcept {
  if (msg == nullptr) {
    return;
  }

  switch (level) {
    case ::opentelemetry::sdk::common::internal_log::LogLevel::Error: {
      TRPC_LOG_ERROR("[OpenTelemetry Error][" << file << ":" << line << "]: " << msg);
      break;
    }
    case ::opentelemetry::sdk::common::internal_log::LogLevel::Info: {
      TRPC_LOG_INFO("[OpenTelemetry Info][" << file << ":" << line << "]: " << msg);
      break;
    }
    case ::opentelemetry::sdk::common::internal_log::LogLevel::Debug: {
      TRPC_LOG_DEBUG("[OpenTelemetry Debug][" << file << ":" << line << "]: " << msg);
      break;
    }
    default: {
      break;
    }
  }
}

}  // namespace trpc::opentelemetry
