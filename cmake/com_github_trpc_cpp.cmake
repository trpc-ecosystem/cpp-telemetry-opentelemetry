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

set(TRPC_CPP_GIT_URL https://github.com/trpc-group/trpc-cpp.git)
set(TRPC_CPP_GIT_TAG main)

FetchContent_Declare(
    trpc-cpp
    GIT_REPOSITORY        ${TRPC_CPP_GIT_URL}
    GIT_TAG               ${TRPC_CPP_GIT_TAG}
    SOURCE_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/cmake_third_party/trpc-cpp
)

FetchContent_GetProperties(trpc-cpp)
if(NOT trpc-cpp_POPULATED)
    FetchContent_Populate(trpc-cpp)

    add_subdirectory(${trpc-cpp_SOURCE_DIR})
endif()

set(TARGET_INCLUDE_PATHS ${TARGET_INCLUDE_PATHS} ${trpc-cpp_SOURCE_DIR})

set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} trpc)
