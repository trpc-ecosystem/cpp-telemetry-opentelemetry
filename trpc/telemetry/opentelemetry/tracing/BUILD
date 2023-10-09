load("@trpc_cpp//trpc:trpc.bzl", "trpc_proto_library")

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    srcs = ["common.cc"],
    hdrs = ["common.h"],
    deps = [
        "@io_opentelemetry_cpp//api",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/server:server_context",
        "@trpc_cpp//trpc/util:pb2json",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_test(
    name = "common_test",
    srcs = ["common_test.cc"],
    deps = [
        ":common",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/proto/testing:cc_helloworld_proto",
    ],
)

cc_library(
    name = "deferred_sample_processor",
    srcs = ["deferred_sample_processor.cc"],
    hdrs = ["deferred_sample_processor.h"],
    deps = [
        "@io_opentelemetry_cpp//sdk/src/trace",
    ],
)

cc_test(
    name = "deferred_sample_processor_test",
    srcs = ["deferred_sample_processor_test.cc"],
    deps = [
        ":deferred_sample_processor",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@io_opentelemetry_cpp//exporters/ostream:ostream_span_exporter",
    ],
)

cc_library(
    name = "sampler",
    srcs = ["sampler.cc"],
    hdrs = ["sampler.h"],
    deps = [
        ":common",
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//sdk/src/trace",
    ],
)

cc_test(
    name = "sampler_test",
    srcs = ["sampler_test.cc"],
    deps = [
        ":sampler",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "text_map_carrier",
    srcs = ["text_map_carrier.cc"],
    hdrs = ["text_map_carrier.h"],
    deps = [
        "@com_github_tencent_rapidjson//:rapidjson",
        "@com_google_protobuf//:protobuf",
        "@io_opentelemetry_cpp//api",
        "@trpc_cpp//trpc/util/http:request",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_test(
    name = "text_map_carrier_test",
    srcs = ["text_map_carrier_test.cc"],
    deps = [
        ":text_map_carrier",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

trpc_proto_library(
    name = "trace_service",
    srcs = [],
    generate_new_mock_code = True,
    native_cc_proto_deps = [
        "@com_github_opentelemetry_proto//:trace_service_proto_cc",
    ],
    native_proto_deps = [
    ],
    rootpath = "@trpc_cpp",
    use_trpc_plugin = True,
    deps = [],
)

cc_library(
    name = "grpc_trace_exporter",
    srcs = ["grpc_trace_exporter.cc"],
    hdrs = ["grpc_trace_exporter.h"],
    deps = [
        ":common",
        ":trace_service",
        "@io_opentelemetry_cpp//api",
        "@io_opentelemetry_cpp//exporters/otlp:otlp_recordable",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@trpc_cpp//trpc/client:make_client_context",
        "@trpc_cpp//trpc/client:trpc_client",
    ],
)

cc_test(
    name = "grpc_trace_exporter_test",
    srcs = ["grpc_trace_exporter_test.cc"],
    deps = [
        ":grpc_trace_exporter",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
    ],
)

cc_library(
    name = "opentelemetry_tracing",
    srcs = ["opentelemetry_tracing.cc"],
    hdrs = ["opentelemetry_tracing.h"],
    deps = [
        ":common",
        ":deferred_sample_processor",
        ":grpc_trace_exporter",
        ":sampler",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf_parser",
        "@io_opentelemetry_cpp//exporters/otlp:otlp_http_exporter",
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@trpc_cpp//trpc/tracing",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_test(
    name = "opentelemetry_tracing_test",
    srcs = ["opentelemetry_tracing_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    deps = [
        ":opentelemetry_tracing",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common:trpc_plugin",
    ],
)

cc_library(
    name = "server_filter",
    srcs = ["server_filter.cc"],
    hdrs = ["server_filter.h"],
    deps = [
        ":common",
        ":opentelemetry_tracing",
        ":text_map_carrier",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@trpc_cpp//trpc/codec/http:http_protocol",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/filter",
        "@trpc_cpp//trpc/server:server_context",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/tracing:tracing_filter_index",
    ],
)

cc_test(
    name = "server_filter_test",
    srcs = ["server_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    deps = [
        ":server_filter",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
        "@trpc_cpp//trpc/codec/trpc/testing:trpc_protocol_testing",
        "@trpc_cpp//trpc/proto/testing:cc_helloworld_proto",
        "@trpc_cpp//trpc/server:http_service",
        "@trpc_cpp//trpc/server:service_adapter",
        "@trpc_cpp//trpc/server/rpc:rpc_service_impl",
        "@trpc_cpp//trpc/server/testing:server_context_testing",
    ],
)

cc_library(
    name = "client_filter",
    srcs = ["client_filter.cc"],
    hdrs = ["client_filter.h"],
    deps = [
        ":common",
        ":opentelemetry_tracing",
        ":text_map_carrier",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/codec/http:http_protocol",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/filter",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/tracing:tracing_filter_index",
    ],
)

cc_test(
    name = "client_filter_test",
    srcs = ["client_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    deps = [
        ":client_filter",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)

cc_library(
    name = "opentelemetry_tracing_api",
    srcs = ["opentelemetry_tracing_api.cc"],
    hdrs = ["opentelemetry_tracing_api.h"],
    deps = [
        ":client_filter",
        ":common",
        ":opentelemetry_tracing",
        ":server_filter",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@trpc_cpp//trpc/server:server_context",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/tracing:tracing_filter_index",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_test(
    name = "opentelemetry_tracing_api_test",
    srcs = ["opentelemetry_tracing_api_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    deps = [
        ":opentelemetry_tracing_api",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "//trpc/telemetry/opentelemetry/tracing:opentelemetry_tracing",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
        "@trpc_cpp//trpc/codec/trpc/testing:trpc_protocol_testing",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/proto/testing:cc_helloworld_proto",
        "@trpc_cpp//trpc/server/rpc:rpc_service_impl",
        "@trpc_cpp//trpc/server/testing:server_context_testing",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/tracing:tracing_filter_index",
    ],
)
