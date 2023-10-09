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

#include "trpc/telemetry/opentelemetry/metrics/opentelemetry_metrics_api.h"
#include "trpc/telemetry/opentelemetry/tracing/opentelemetry_tracing_api.h"

/// @brief OpenTelemetry plugin interfaces for user programing
namespace trpc::opentelemetry {

/// @brief Initializes the OpenTelemetry Telemetry plugin and filters.
bool Init();

}  // namespace trpc::opentelemetry
