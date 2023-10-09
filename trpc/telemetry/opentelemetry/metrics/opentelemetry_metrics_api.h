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

#ifdef TRPC_BUILD_INCLUDE_PROMETHEUS
#pragma once

#include <map>
#include <string>

#include "trpc/metrics/metrics.h"

/// @brief OpenTelemetry metrics interfaces for user programing
namespace trpc::opentelemetry {

/// @brief Reports metrics data with SUM type
/// @param labels metrics labels
/// @param value the value to increment
/// @return Return 0 for success and non-zero for failure.
int ReportSumMetricsInfo(const std::map<std::string, std::string>& labels, double value);

/// @brief Reports metrics data with SET type
/// @param labels metrics labels
/// @param value the value to set
/// @return Return 0 for success and non-zero for failure.
int ReportSetMetricsInfo(const std::map<std::string, std::string>& labels, double value);

/// @brief Reports metrics data with MID type
/// @param labels metrics labels
/// @param value the value to observe
/// @return Return 0 for success and non-zero for failure.
int ReportMidMetricsInfo(const std::map<std::string, std::string>& labels, double value);

/// @brief Reports metrics data with QUANTILES type
/// @param labels metrics labels
/// @param quantiles the quantiles used to gather summary statistics
/// @param value the value to observe
/// @return Return 0 for success and non-zero for failure.
int ReportQuantilesMetricsInfo(const std::map<std::string, std::string>& labels, const SummaryQuantiles& quantiles,
                               double value);

/// @brief Reports metrics data with HISTOGRAM type
/// @param labels metrics labels
/// @param bucket the bucket used to gather histogram statistics
/// @param value the value to observe
/// @return Return 0 for success and non-zero for failure.
int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, const HistogramBucket& bucket,
                               double value);
int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, HistogramBucket&& bucket,
                               double value);

}  // namespace trpc::opentelemetry
#endif
