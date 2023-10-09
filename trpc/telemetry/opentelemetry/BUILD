licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "opentelemetry_common",
    hdrs = ["opentelemetry_common.h"],
    deps = [],
)

cc_library(
    name = "opentelemetry_log_handler",
    srcs = ["opentelemetry_log_handler.cc"],
    hdrs = ["opentelemetry_log_handler.h"],
    deps = [
        "@io_opentelemetry_cpp//sdk/src/trace",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_library(
    name = "opentelemetry_telemetry_conf",
    srcs = ["opentelemetry_telemetry_conf.cc"],
    hdrs = ["opentelemetry_telemetry_conf.h"],
    deps = [
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_library(
    name = "opentelemetry_telemetry_conf_parser",
    hdrs = ["opentelemetry_telemetry_conf_parser.h"],
    deps = [
        ":opentelemetry_telemetry_conf",
    ],
)

cc_test(
    name = "opentelemetry_telemetry_conf_test",
    srcs = ["opentelemetry_telemetry_conf_test.cc"],
    deps = [
        ":opentelemetry_telemetry_conf",
        ":opentelemetry_telemetry_conf_parser",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "opentelemetry_telemetry",
    srcs = ["opentelemetry_telemetry.cc"],
    hdrs = ["opentelemetry_telemetry.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_log_handler",
        "opentelemetry_common",
        "opentelemetry_telemetry_conf",
        "opentelemetry_telemetry_conf_parser",
        "//trpc/telemetry/opentelemetry/logging:opentelemetry_logging",
        "//trpc/telemetry/opentelemetry/tracing:opentelemetry_tracing",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/telemetry:telemetry",
        "@trpc_cpp//trpc/util/log/default:default_log",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:opentelemetry_metrics",
        ],
        "//trpc:include_metrics_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:opentelemetry_metrics",
        ],
    }),
)

cc_test(
    name = "opentelemetry_telemetry_test",
    srcs = ["opentelemetry_telemetry_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/util/log:logging",
    ],
)

cc_library(
    name = "opentelemetry_server_filter",
    srcs = ["opentelemetry_server_filter.cc"],
    hdrs = ["opentelemetry_server_filter.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        "//trpc/telemetry/opentelemetry/tracing:server_filter",
        "@trpc_cpp//trpc/filter",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:server_filter",
        ],
        "//trpc:include_metrics_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:server_filter",
        ],
    }),
)

cc_test(
    name = "opentelemetry_server_filter_test",
    srcs = ["opentelemetry_server_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_server_filter",
        ":opentelemetry_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
        "@trpc_cpp//trpc/codec/trpc/testing:trpc_protocol_testing",
        "@trpc_cpp//trpc/proto/testing:cc_helloworld_proto",
        "@trpc_cpp//trpc/server:http_service",
        "@trpc_cpp//trpc/server:service_adapter",
        "@trpc_cpp//trpc/server/testing:server_context_testing",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)

cc_library(
    name = "opentelemetry_client_filter",
    srcs = ["opentelemetry_client_filter.cc"],
    hdrs = ["opentelemetry_client_filter.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        "//trpc/telemetry/opentelemetry/tracing:client_filter",
        "@trpc_cpp//trpc/filter",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:client_filter",
        ],
        "//trpc:include_metrics_prometheus": [
            "//trpc/telemetry/opentelemetry/metrics:client_filter",
        ],
    }),
)

cc_test(
    name = "opentelemetry_client_filter_test",
    srcs = ["opentelemetry_client_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_client_filter",
        ":opentelemetry_server_filter",
        ":opentelemetry_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)

cc_library(
    name = "opentelemetry_telemetry_api",
    srcs = ["opentelemetry_telemetry_api.cc"],
    hdrs = ["opentelemetry_telemetry_api.h"],
    deps = [
        ":opentelemetry_client_filter",
        ":opentelemetry_server_filter",
        ":opentelemetry_telemetry",
        "//trpc/telemetry/opentelemetry/metrics:opentelemetry_metrics_api",
        "//trpc/telemetry/opentelemetry/tracing:opentelemetry_tracing_api",
        "@trpc_cpp//trpc/common:trpc_plugin",
    ],
)
