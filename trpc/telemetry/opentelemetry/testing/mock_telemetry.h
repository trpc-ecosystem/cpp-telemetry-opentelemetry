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

#include "gmock/gmock.h"

#include "trpc/telemetry/opentelemetry/opentelemetry_common.h"
#include "trpc/telemetry/telemetry.h"

namespace trpc::testing {

class MockOpenTelemetryTelemetry : public Telemetry {
 public:
  std::string Name() const override { return trpc::opentelemetry::kOpenTelemetryTelemetryName; }

  MOCK_METHOD(TracingPtr, GetTracing, (), (override));

  MOCK_METHOD(MetricsPtr, GetMetrics, (), (override));

  MOCK_METHOD(LoggingPtr, GetLog, (), (override));
};

}  // namespace trpc::testing
