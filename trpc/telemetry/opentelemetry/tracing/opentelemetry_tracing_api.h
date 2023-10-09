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

#include <cstdint>

#include "trpc/server/server_context.h"

#include "trpc/telemetry/opentelemetry/tracing/client_filter.h"
#include "trpc/telemetry/opentelemetry/tracing/common.h"
#include "trpc/telemetry/opentelemetry/tracing/server_filter.h"

/// @brief OpenTelemetry tracing interfaces for user programing
namespace trpc::opentelemetry {

/// @brief Definition of an invalid index returned by the GetTracingFilterDataIndex interface.
constexpr uint32_t kInvalidTracingFilterDataIndex = 0;

/// @brief Gets the tracing filter data index of the OpenTelemetry plugin, which can be used to get or set tracing data.
/// @return Return the tracing filter data index. Note that kInvalidTracingFilterDataIndex will be returned when the
///         OpenTelemetry plugin is not registered correctly.
uint32_t GetTracingFilterDataIndex();

/// @brief Gets the span.
/// @param context server context
/// @return Return the span saved in the context. Note that OpenTelemetryTracingSpanPtr(nullptr) will be returned when
///         there is no valid span in the context.
OpenTelemetryTracingSpanPtr GetTracingSpan(const ServerContextPtr& context);

/// @brief Gets the trace id.
/// @param context server context
/// @return Return the trace id of the context. Note that empty string will be return when there is no valid span in the
///         context.
std::string GetTraceID(const ServerContextPtr& context);

/// @brief Gets the span id.
/// @param context server context
/// @return Return the span id of the context. Note that empty string will be return when there is no valid span in the
///         context.
std::string GetSpanID(const ServerContextPtr& context);

}  // namespace trpc::opentelemetry
