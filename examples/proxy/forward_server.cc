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

#include <memory>
#include <string>

#include "fmt/format.h"
#include "trpc/common/trpc_app.h"

#include "examples/proxy/forward_service.h"
#include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

namespace examples::forward {

void ServerTraceAttributes(const trpc::ServerContextPtr& context, const void* req,
                           std::unordered_map<std::string, std::string>& attributes) {
  if (context->GetFuncName() == "/trpc.test.route.Forward/Route") {
    auto hello_request = static_cast<const ::trpc::test::helloworld::HelloRequest*>(req);
    if (hello_request->msg() == "force") {
      attributes[::trpc::opentelemetry::kForceSampleKey] = "sample";
    }
  }
}

class ForwardServer : public ::trpc::TrpcApp {
 public:
  int Initialize() override {
    const auto& config = ::trpc::TrpcConfig::GetInstance()->GetServerConfig();
    // Set the service name, which must be the same as the value of the `server:service:name` configuration item
    // in the framework configuration file, otherwise the framework cannot receive requests normally
    std::string service_name = fmt::format("{}.{}.{}.{}", "trpc", config.app, config.server, "Forward");

    TRPC_FMT_INFO("service name:{}", service_name);

    RegisterService(service_name, std::make_shared<ForwardServiceImpl>());

    return 0;
  }

  void Destroy() override {}

  int RegisterPlugins() override {
    // Initializes the OpenTelemetry plugin and filters in RegisterPlugins
    ::trpc::opentelemetry::Init();
    // Sets the SetServerTraceAttrsFunc
    ::trpc::opentelemetry::SetServerTraceAttrsFunc(ServerTraceAttributes);
    return 0;
  }
};

}  // namespace examples::forward

int main(int argc, char** argv) {
  examples::forward::ForwardServer forward_server;
  forward_server.Main(argc, argv);
  forward_server.Wait();

  return 0;
}
