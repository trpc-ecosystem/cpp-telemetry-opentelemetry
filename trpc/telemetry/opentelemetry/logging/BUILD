load("@trpc_cpp//trpc:trpc.bzl", "trpc_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    hdrs = ["common.h"],
    deps = [],
)

trpc_proto_library(
    name = "logs_service",
    srcs = [],
    generate_new_mock_code = True,
    native_cc_proto_deps = [
        "@com_github_opentelemetry_proto//:logs_service_proto_cc",
    ],
    native_proto_deps = [
    ],
    rootpath = "@trpc_cpp",
    use_trpc_plugin = True,
    deps = [],
)

cc_library(
    name = "grpc_log_exporter",
    srcs = ["grpc_log_exporter.cc"],
    hdrs = ["grpc_log_exporter.h"],
    deps = [
        ":common",
        ":logs_service",
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//exporters/otlp:otlp_recordable",
        "@io_opentelemetry_cpp//sdk/src/logs",
        "@trpc_cpp//trpc/client:make_client_context",
        "@trpc_cpp//trpc/client:trpc_client",
    ],
)

cc_test(
    name = "grpc_log_exporter_test",
    srcs = ["grpc_log_exporter_test.cc"],
    deps = [
        ":grpc_log_exporter",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
    ],
)

cc_library(
    name = "opentelemetry_logging",
    srcs = ["opentelemetry_logging.cc"],
    hdrs = ["opentelemetry_logging.h"],
    deps = [
        ":grpc_log_exporter",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf_parser",
        "//trpc/telemetry/opentelemetry/tracing:opentelemetry_tracing_api",
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//exporters/otlp:otlp_http_log_record_exporter",
        "@io_opentelemetry_cpp//sdk/src/logs",
        "@trpc_cpp//trpc/log:logging",
        "@trpc_cpp//trpc/tracing:tracing_filter_index",
    ],
)

cc_test(
    name = "opentelemetry_logging_test",
    srcs = ["opentelemetry_logging_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    deps = [
        ":opentelemetry_logging",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)
