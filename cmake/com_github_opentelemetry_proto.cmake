include(FetchContent)

set(OPENTELEMETRY_PROTO_GIT_URL https://github.com/open-telemetry/opentelemetry-proto.git)
set(OPENTELEMETRY_PROTO_GIT_TAG v0.19.0)

FetchContent_Declare(
    com_github_opentelemtry_proto
    GIT_REPOSITORY        ${OPENTELEMETRY_PROTO_GIT_URL}
    GIT_TAG               ${OPENTELEMETRY_PROTO_GIT_TAG}
    SOURCE_DIR            ${CMAKE_CURRENT_SOURCE_DIR}/cmake_third_party/com_github_opentelemetry_proto
)

FetchContent_GetProperties(com_github_opentelemtry_proto)
if(NOT com_github_opentelemtry_proto_POPULATED)
    FetchContent_Populate(com_github_opentelemtry_proto)
    # Generating pb.h and pb.cc using cmake function provided in tRPC-Cpp
    file(GLOB_RECURSE OPENTELEMETRY_PROTO_SRC_FILES ${com_github_opentelemtry_proto_SOURCE_DIR}/*.proto)
    COMPILE_PROTO(OPENTELEMETRY_PROTO_OUT_PB_SRCS_FILES
                                "${OPENTELEMETRY_PROTO_SRC_FILES}"
                                ${PROTOBUF_PROTOC_EXECUTABLE}
                                ${com_github_opentelemtry_proto_SOURCE_DIR})
    add_library(trpc_opentelemetry_proto ${OPENTELEMETRY_PROTO_OUT_PB_SRCS_FILES})
    target_link_libraries(trpc_opentelemetry_proto trpc_protobuf)
    target_include_directories(trpc_opentelemetry_proto PUBLIC ${com_github_opentelemtry_proto_SOURCE_DIR})
    set(TARGET_LINK_LIBS ${TARGET_LINK_LIBS} trpc_opentelemetry_proto)
    set(TARGET_INCLUDE_PATHS  ${TARGET_INCLUDE_PATHS} ${com_github_opentelemtry_proto_SOURCE_DIR})
    # Generating trpc.pb.h and trpc.pb.cc using tools provided in tRPC-Cpp
    set(OUT_OPENTELEMETRY_PROTO_TRPC_PB_PROTO_FILES 
                                ${CMAKE_CURRENT_SOURCE_DIR}/trpc/telemetry/opentelemetry/logging/logs_service.trpc.pb.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/trpc/telemetry/opentelemetry/logging/logs_service.trpc.pb.cc
                                ${CMAKE_CURRENT_SOURCE_DIR}/trpc/telemetry/opentelemetry/tracing/trace_service.trpc.pb.h
                                ${CMAKE_CURRENT_SOURCE_DIR}/trpc/telemetry/opentelemetry/tracing/trace_service.trpc.pb.cc)
    add_custom_command(
        OUTPUT ${OUT_OPENTELEMETRY_PROTO_TRPC_PB_PROTO_FILES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=cmake_third_party/com_github_opentelemetry_proto
            --plugin=protoc-gen-trpc=${TRPC_TO_CPP_PLUGIN}
            --trpc_out=generate_trpc_stub_path=trpc/telemetry/opentelemetry/logging/logs_service:.
            -I. -Icmake_third_party/com_github_opentelemetry_proto
            cmake_third_party/com_github_opentelemetry_proto/opentelemetry/proto/collector/logs/v1/logs_service.proto
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} --proto_path=cmake_third_party/com_github_opentelemetry_proto
            --plugin=protoc-gen-trpc=${TRPC_TO_CPP_PLUGIN}
            --trpc_out=generate_trpc_stub_path=trpc/telemetry/opentelemetry/tracing/trace_service:.
            -I. -Icmake_third_party/com_github_opentelemetry_proto
            cmake_third_party/com_github_opentelemetry_proto/opentelemetry/proto/collector/trace/v1/trace_service.proto
        DEPENDS ${com_github_opentelemtry_proto_SOURCE_DIR}/opentelemetry/proto/collector/logs/v1/logs_service.proto
            ${com_github_opentelemtry_proto_SOURCE_DIR}/opentelemetry/proto/collector/trace/v1/trace_service.proto
            trpc_cpp_plugin
    )
endif()