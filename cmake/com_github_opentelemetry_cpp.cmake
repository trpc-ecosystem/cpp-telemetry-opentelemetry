#
#
# Tencent is pleased to support the open source community by making tRPC available.
#
# Copyright (C) 2023 THL A29 Limited, a Tencent company.
# All rights reserved.
#
# If you have downloaded a copy of the tRPC source code from Tencent,
# please note that tRPC source code is licensed under the  Apache 2.0 License,
# A copy of the Apache 2.0 License is included in this file.
#
#

include(FetchContent)

set(OPEN_TELEMETRY_TAG v1.9.1)
set(OPEN_TELEMETRY_URL https://github.com/open-telemetry/opentelemetry-cpp/archive/${OPEN_TELEMETRY_TAG}.tar.gz)

FetchContent_Declare(
    com_github_opentelemetry_cpp
    URL                   ${OPEN_TELEMETRY_URL}
    SOURCE_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/cmake_third_party/com_github_opentelemetry_cpp
)

FetchContent_GetProperties(com_github_opentelemetry_cpp)
if(NOT com_github_opentelemetry_cpp_POPULATED)
    FetchContent_Populate(com_github_opentelemetry_cpp)

    set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
    set(WITH_OTLP ON)
    set(WITH_OTLP_HTTP ON)
    set(WITH_EXAMPLES OFF)
    set(WITH_FUNC_TESTS OFF)
    set(BUILD_TESTING OFF)
    set(WITH_BENCHMARK OFF)
    set(WITH_LOGS_PREVIEW ON)

    add_subdirectory(${com_github_opentelemetry_cpp_SOURCE_DIR})
endif()

set(TARGET_INCLUDE_PATHS  ${TARGET_INCLUDE_PATHS}
                          ${com_github_opentelemetry_cpp_SOURCE_DIR}/api/include
                          ${com_github_opentelemetry_cpp_SOURCE_DIR}/sdk/include
                          ${com_github_opentelemetry_cpp_SOURCE_DIR}/ext/include
                          ${com_github_opentelemetry_cpp_SOURCE_DIR}/exporters/otlp/include)

set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS}
                     opentelemetry_sdk
                     opentelemetry_api
                     opentelemetry_trace
                     opentelemetry_ext
                     opentelemetry_otlp_recordable
                     opentelemetry_exporter_otlp_http_client
                     opentelemetry_exporter_otlp_http
                     opentelemetry_exporter_otlp_http_log
                     opentelemetry_exporter_otlp_http_metric)
