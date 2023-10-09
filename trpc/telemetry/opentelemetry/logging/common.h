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

namespace trpc::opentelemetry {

/// @brief The definitions of resource/attribute keys used in log reporting.
constexpr char kLogServerName[] = "server";
constexpr char kLogServiceName[] = "service.name";
constexpr char kLogEnvName[] = "env";

/// @brief The service name of the internal log exporter
constexpr char kGrpcLogExporterServiceName[] = "trpc.opentelemetry.log.grpc_exporter";

}  // namespace trpc::opentelemetry
