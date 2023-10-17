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

cmake_minimum_required(VERSION 3.14)

project(trpc-cpp-plugin-telemetry-opentelemetry)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

message("Cloning trpc-cpp, please waiting...")
include(com_github_trpc_cpp)

message("Cloning opentelemetry_cpp sdk, please waiting...")
include(com_github_opentelemetry_cpp)

message("Cloning opentelemetry_proto, please waitint...")
include(com_github_opentelemetry_proto)

# Build plugin
file(GLOB_RECURSE SRC_FILES ./trpc/*.cc)

file(GLOB_RECURSE TEST_FILES ./trpc/*test.cc
                             ./trpc/telemetry/opentelemetry/testing/*)

list(REMOVE_ITEM SRC_FILES ${TEST_FILES})

add_library(trpc_cpp_plugin_telemetry_opentelemetry
              ${SRC_FILES}
              ${OUT_OPENTELEMETRY_PROTO_TRPC_PB_PROTO_FILES})

target_link_libraries(
  trpc_cpp_plugin_telemetry_opentelemetry PRIVATE
  ${TARGET_LINK_LIBS}
)

target_include_directories(
  trpc_cpp_plugin_telemetry_opentelemetry PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${TARGET_INCLUDE_PATHS}
)
