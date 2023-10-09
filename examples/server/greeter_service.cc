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

#include "examples/server/greeter_service.h"

#include <string>

#include "trpc/log/trpc_log.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

namespace test {
namespace helloworld {

::trpc::Status GreeterServiceImpl::SayHello(::trpc::ServerContextPtr context,
                                            const ::trpc::test::helloworld::HelloRequest* request,
                                            ::trpc::test::helloworld::HelloReply* reply) {
  // gets the current span and performs some operation provided by the OpenTelemetry API
  ::trpc::opentelemetry::OpenTelemetryTracingSpanPtr span = ::trpc::opentelemetry::GetTracingSpan(context);
  if (span.get()) {
    TRPC_FMT_INFO("Add info to OpenTelemetry span");
    span->SetAttribute("spankey", "spanvalue");
    span->AddEvent("eventkey", {{"int value", 8888}, {"string value", "test"}});
  }
  // gets the trace id and span id
  TRPC_FMT_INFO("the OpenTelemetry trace id is {}", ::trpc::opentelemetry::GetTraceID(context));
  TRPC_FMT_INFO("the OpenTelemetry span id is {}", ::trpc::opentelemetry::GetSpanID(context));

  std::string response = "Hello, " + request->msg();
  reply->set_msg(response);

  return ::trpc::kSuccStatus;
}

}  // namespace helloworld
}  // namespace test
