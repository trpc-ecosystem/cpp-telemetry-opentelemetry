licenses(["notice"])

package(default_visibility = ["//visibility:public"])

exports_files(["opentelemetry_telemetry_test.yaml"])

cc_library(
    name = "mock_telemetry",
    hdrs = ["mock_telemetry.h"],
    deps = [
        "//trpc/telemetry/opentelemetry:opentelemetry_common",
        "@com_google_googletest//:gtest",
        "@trpc_cpp//trpc/telemetry",
    ],
)
