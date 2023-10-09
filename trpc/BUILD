# Description: cpp-telemetry-opentelemetry.

licenses(["notice"])

package(default_visibility = ["//visibility:public"])

# Enable prometheus by setting this compiling option. Default close.
config_setting(
    name = "trpc_include_prometheus",
    values = {"define": "trpc_include_prometheus=true"},
)

# This setting exists to maintain compatibility with old usage. Please use "trpc_include_prometheus" instead.
config_setting(
    name = "include_metrics_prometheus",
    values = {"define": "include_metrics_prometheus=true"},
)
