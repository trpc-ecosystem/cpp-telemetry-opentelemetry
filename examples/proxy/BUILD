load("@trpc_cpp//trpc:trpc.bzl", "trpc_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

trpc_proto_library(
    name = "forward_proto",
    srcs = ["forward.proto"],
    rootpath = "@trpc_cpp",
    use_trpc_plugin = True,
    deps = [
        "//examples/server:helloworld_proto",
    ],
)

cc_binary(
    name = "forward_server",
    srcs = ["forward_server.cc"],
    deps = [
        ":forward_service",
        "@trpc_cpp//trpc/common:trpc_app",
    ],
)

cc_library(
    name = "forward_service",
    srcs = ["forward_service.cc"],
    hdrs = ["forward_service.h"],
    deps = [
        ":forward_proto",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_api",
        "@trpc_cpp//trpc/client:make_client_context",
        "@trpc_cpp//trpc/client:trpc_client",
        "@trpc_cpp//trpc/log:trpc_log",
    ],
)
