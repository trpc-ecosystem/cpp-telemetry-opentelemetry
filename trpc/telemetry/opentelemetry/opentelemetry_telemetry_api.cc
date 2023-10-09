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

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

#include "trpc/common/trpc_plugin.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_client_filter.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_server_filter.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry.h"

namespace trpc::opentelemetry {

bool Init() {
  TrpcPlugin::GetInstance()->RegisterTelemetry(MakeRefCounted<OpenTelemetryTelemetry>());
  TrpcPlugin::GetInstance()->RegisterServerFilter(std::make_shared<OpenTelemetryServerFilter>());
  TrpcPlugin::GetInstance()->RegisterClientFilter(std::make_shared<OpenTelemetryClientFilter>());
  return true;
}

}  // namespace trpc::opentelemetry
