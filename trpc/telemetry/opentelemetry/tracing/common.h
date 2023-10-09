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

#include <functional>
#include <unordered_map>

#include "google/protobuf/message.h"
#include "opentelemetry/trace/span.h"
#include "trpc/client/client_context.h"
#include "trpc/server/server_context.h"

namespace trpc::opentelemetry {

/// @brief The opentelemetry tracing data that saves in the context for transmission
using OpenTelemetryTracingSpanPtr = ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>;

/// @brief The definitions of resource/attribute keys used in tracing reporting.
constexpr char kTraceServiceName[] = "service.name";
constexpr char kTracePeerIp[] = "net.peer.ip";
constexpr char kTracePeerPort[] = "net.peer.port";
constexpr char kTraceHostIp[] = "net.host.ip";
constexpr char kTraceHostPort[] = "net.host.port";
constexpr char kTraceCallerService[] = "trpc.caller_service";
constexpr char kTraceCallerMethod[] = "trpc.caller_method";
constexpr char kTraceCalleeService[] = "trpc.callee_service";
constexpr char kTraceCalleeMethod[] = "trpc.callee_method";
constexpr char kTraceNamespace[] = "trpc.namespace";
constexpr char kTraceEnvName[] = "trpc.envname";
constexpr char kTraceDyeingKey[] = "trpc.dyeing_key";
constexpr char kTraceFrameworkRetCode[] = "trpc.framework_ret";
constexpr char kTraceFuncRetCode[] = "trpc.func_ret";
constexpr char kTraceErrMsg[] = "trpc.err_msg";

/// @brief The service name of the internal trace exporter
constexpr char kGrpcTraceExporterServiceName[] = "trpc.opentelemetry.trace.grpc_exporter";

/// @brief The flag for forced sampling.
static constexpr char kForceSampleKey[] = "trpc-force-sample";

/// @brief The type definition of the span's startup attributes setting function. Users can customize startup attributes
///        through this callback function.
/// @param context The context of this call
/// @param req Pointer to request data, which can be converted to a pointer of a specific request type.
/// @param [out] attributes Span's startup attributes. It will be passed as the attributes parameter when creating a
///                         Span, and can be used in the Sampler. It will ultimately be reflected in the attributes of
///                         the reported Span.
using ServerTraceAttributesFunc = std::function<void(const trpc::ServerContextPtr& context, const void* req,
                                                     std::unordered_map<std::string, std::string>& attributes)>;
using ClientTraceAttributesFunc = std::function<void(const trpc::ClientContextPtr& context, const void* req,
                                                     std::unordered_map<std::string, std::string>& attributes)>;

/// @brief The default maximum length of the request/response data that allowed to be reported.
const uint32_t kDefaultMaxStringLength = 32766;

/// @brief The suffix to be appended to the truncated request/response data in case they exceed the maximum
///        allowed length.
constexpr char kFixedStringSuffix[] = "...stringLengthTooLong";

/// @brief Gets the maximum allowed length of the request/response data that can be reported
uint32_t GetMaxStringLength();

/// @brief Sets the maximum allowed length of the request/response data that can be reported
/// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
void SetMaxStringLength(uint32_t limit);

/// @brief Gets server-side span's startup attributes setting function
ServerTraceAttributesFunc GetServerTraceAttrsFunc();

/// @brief Sets server-side span's startup attributes setting function
/// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
void SetServerTraceAttrsFunc(ServerTraceAttributesFunc func);

/// @brief Gets client-side span's startup attributes setting function
ClientTraceAttributesFunc GetClientTraceAttrsFunc();

/// @brief Sets client-side span's startup attributes setting function
/// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
void SetClientTraceAttrsFunc(ClientTraceAttributesFunc func);

namespace detail {

/// @brief Converts protobuf::Message to json format.
/// @param msg request/response
/// @param [out] json_data the json formatted data after conversion
void GetMsgJsonData(const google::protobuf::Message* pb_msg, std::string& json_data);

/// @brief Sets the status info into span
template <typename Context>
void SetStatus(const Context& context, const OpenTelemetryTracingSpanPtr& span) {
  if (context->GetStatus().OK()) {
    span->SetStatus(::opentelemetry::trace::StatusCode::kOk, "");
  } else {
    span->SetAttribute(kTraceFrameworkRetCode, context->GetStatus().GetFrameworkRetCode());
    span->SetAttribute(kTraceFuncRetCode, context->GetStatus().GetFuncRetCode());
    span->SetAttribute(kTraceErrMsg, context->GetStatus().ErrorMessage());
    span->SetStatus(::opentelemetry::trace::StatusCode::kError, context->GetStatus().ErrorMessage());
  }
}

}  // namespace detail

}  // namespace trpc::opentelemetry
