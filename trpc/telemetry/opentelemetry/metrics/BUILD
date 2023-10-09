licenses(["notice"])

package(default_visibility = ["//visibility:public"])

cc_library(
    name = "common",
    srcs = ["common.cc"],
    hdrs = ["common.h"],
    deps = [
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf",
        "@trpc_cpp//trpc/codec:codec_helper",
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
    ],
)

cc_library(
    name = "opentelemetry_metrics",
    srcs = ["opentelemetry_metrics.cc"],
    hdrs = ["opentelemetry_metrics.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":common",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf",
        "//trpc/telemetry/opentelemetry:opentelemetry_telemetry_conf_parser",
        "@trpc_cpp//trpc/util:prometheus",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/metrics",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
        "//trpc:include_metrics_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
    }),
)

cc_test(
    name = "opentelemetry_metrics_test",
    srcs = ["opentelemetry_metrics_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_metrics",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "server_filter",
    srcs = ["server_filter.cc"],
    hdrs = ["server_filter.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":common",
        ":opentelemetry_metrics",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@trpc_cpp//trpc/filter:filter",
        "@trpc_cpp//trpc/server:server_context",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/util:time",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
        "//trpc:include_metrics_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
    }),
)

cc_test(
    name = "server_filter_test",
    srcs = ["server_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":server_filter",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/client/testing:service_proxy_testing",
        "@trpc_cpp//trpc/codec/trpc/testing:trpc_protocol_testing",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/proto/testing:cc_helloworld_proto",
        "@trpc_cpp//trpc/server/rpc:rpc_service_impl",
        "@trpc_cpp//trpc/server/testing:server_context_testing",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)

cc_library(
    name = "client_filter",
    srcs = ["client_filter.cc"],
    hdrs = ["client_filter.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":common",
        ":opentelemetry_metrics",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@trpc_cpp//trpc/filter:filter",
        "@trpc_cpp//trpc/client:client_context",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/util:time",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
        "//trpc:include_metrics_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
    }),
)

cc_test(
    name = "client_filter_test",
    srcs = ["client_filter_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":client_filter",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/codec/trpc:trpc_client_codec",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)

cc_library(
    name = "opentelemetry_metrics_api",
    srcs = ["opentelemetry_metrics_api.cc"],
    hdrs = ["opentelemetry_metrics_api.h"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":common",
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "//trpc/telemetry/opentelemetry/metrics:opentelemetry_metrics",
        "@trpc_cpp//trpc/metrics:metrics",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
        "@trpc_cpp//trpc/util/log:logging",
    ] + select({
        "//conditions:default": [],
        "//trpc:trpc_include_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
        "//trpc:include_metrics_prometheus": [
            "@com_github_jupp0r_prometheus_cpp//pull",
        ],
    }),
)

cc_test(
    name = "opentelemetry_metrics_api_test",
    srcs = ["opentelemetry_metrics_api_test.cc"],
    data = ["//trpc/telemetry/opentelemetry/testing:opentelemetry_telemetry_test.yaml"],
    defines = [] + select({
        "//trpc:trpc_include_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//trpc:include_metrics_prometheus": ["TRPC_BUILD_INCLUDE_PROMETHEUS"],
        "//conditions:default": [],
    }),
    deps = [
        ":opentelemetry_metrics_api",
        "//trpc/telemetry/opentelemetry/metrics:opentelemetry_metrics",
        "//trpc/telemetry/opentelemetry/testing:mock_telemetry",
        "@com_google_googletest//:gtest",
        "@com_google_googletest//:gtest_main",
        "@trpc_cpp//trpc/common/config:trpc_config",
        "@trpc_cpp//trpc/telemetry:telemetry_factory",
    ],
)
