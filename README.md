[中文](./README.zh_CN.md)

[![LICENSE](https://img.shields.io/badge/license-Apache--2.0-green.svg)](https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry/blob/main/LICENSE)
[![Releases](https://img.shields.io/github/release/trpc-ecosystem/cpp-telemetry-opentelemetry.svg?style=flat-square)](https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry/releases)
[![Build Status](https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry/actions/workflows/ci.yml/badge.svg)](https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry/actions/workflows/ci.yml)
[![Coverage](https://codecov.io/gh/trpc-ecosystem/cpp-telemetry-opentelemetry/branch/main/graph/badge.svg)](https://app.codecov.io/gh/trpc-ecosystem/cpp-telemetry-opentelemetry/tree/main)

# Overview

[OpenTelemetry](https://opentelemetry.io/) is a popular observability framework and toolkit used for creating and managing telemetry data such as traces, metrics, and logs. We provide an OpenTelemetry Telemetry plugin that allows users to easily use OpenTelemetry for collecting and reporting telemetry data in tRPC-Cpp.

In the OpenTelemetry Telemetry plugin, the reported traces and logs data are using the OpenTelemetry protocol, while the reporting of metrics data utilizes the prometheus capabilities of the framework, using the [Prometheus](https://prometheus.io/) format.

# Usage

For detailed examples, please refer to the [OpenTelemetry examples](./examples/).

## Integration

To use the OpenTelemetry Telemetry plugin in a project, you need to follow these steps for integration.

### Import dependencies

#### Bazel

1. Import repository

    Import the `cpp-telemetry-opentelemetry` repository and its dependencies in your project's `WORKSPACE` file.

    ```python
    load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

    git_repository(
        name = "trpc_cpp",
        remote = "https://github.com/trpc-group/trpc-cpp.git",
        branch = "main",
    )

    load("@trpc_cpp//trpc:workspace.bzl", "trpc_workspace")
    trpc_workspace()

    git_repository(
        name = "cpp-telemetry-opentelemetry",
        remote = "https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry.git",
        branch = "main",
    )

    load("@cpp-telemetry-opentelemetry//trpc:workspace.bzl", "telemetry_opentelemetry_workspace")
    telemetry_opentelemetry_workspace()
    ```

2. Import plugin

    Import the "`trpc/telemetry/opentelemetry:opentelemetry_telemetry_api`" dependency in the targets that require OpenTelemetry. For example:

    ```python
    cc_binary(
        name = "helloworld_server",
        srcs = ["helloworld_server.cc"],
        deps = [
            "@cpp-telemetry-opentelemetry//trpc/telemetry/opentelemetry:opentelemetry_telemetry_api",
            ...
        ],
    )
    ```

3. Compilation options

    * Since the metrics feature relies on the prometheus capability of the framework, to use the metrics functionality of the plugin, you need to include the "`trpc_include_prometheus`" compilation option. For example, add the following line to your .bazelrc file:

        ```bash
        build --define trpc_include_prometheus=true
        ```

    * Since the logging feature is still in preview in the current version of [opentelemetry-cpp (v1.9.1)](https://github.com/open-telemetry/opentelemetry-cpp/tree/v1.9.1), to use the logging functionality of the plugin, you need to add the "`ENABLE_LOGS_PREVIEW`" compilation macro. For example, add the following line to your .bazelrc file:

        ```bash
        build --copt="-DENABLE_LOGS_PREVIEW"
        ```

#### CMake

Please refer to below code snippets of CMakeLists.txt:

```bash
# Enable promethues
set(TRPC_BUILD_WITH_METRICS_PROMETHEUS ON)

# First, import trpc-cpp.
include(FetchContent)
FetchContent_Declare(
    trpc-cpp
    GIT_REPOSITORY    https://github.com/trpc-group/trpc-cpp.git
    GIT_TAG           change_to_tag_you_use
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/cmake_third_party/trpc-cpp
)
FetchContent_MakeAvailable(trpc-cpp)

# Then, import cpp-telemetry-opentelemetry
FetchContent_Declare(
    trpc_cpp_telemetry_opentelemetry
    GIT_REPOSITORY    https://github.com/trpc-ecosystem/cpp-telemetry-opentelemetry.git
    GIT_TAG           change_to_tag_you_use
    SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}/cmake_third_party/trpc_cpp_telemetry_opentelemetry
)
FetchContent_MakeAvailable(trpc_cpp_telemetry_opentelemetry)

# Last, link to your target
target_link_libraries(your_target trpc
                                  trpc_cpp_plugin_telemetry_opentelemetry)
```

### Registration

The OpenTelemetry plugin provides an interface for registering plugin and filters called `::trpc::opentelemetry::Init`. The users need to call this interface for initialization before starting the framework.

1. For server scenarios, users need to call it in the `TrpcApp::RegisterPlugins` function during service startup:

    ```cpp
    #include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

    class HelloworldServer : public ::trpc::TrpcApp {
     public:
      ...
      int RegisterPlugins() override {
        ::trpc::opentelemetry::Init();
        return 0;
      }
    };
    ```

2. For pure client scenarios, it needs to be called after initializing the framework configuration but before starting other modules of the framework:

    ```cpp
    #include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

    int main(int argc, char* argv[]) {
      ParseClientConfig(argc, argv);

      ::trpc::opentelemetry::Init();

      return ::trpc::RunInTrpcRuntime([]() { return Run(); });
    }
    ```

### Configure the plugin

It necessary to add the configuration of the OpenTelemetry plugin in the framework's configuration file.

```yaml
plugins:
  telemetry:
    opentelemetry:
      addr: 127.0.0.1:4318
      protocol: http
      selector_name: direct
      timeout: 10000
      sampler:
        fraction: 0.001
      traces:
        disable_trace_body: true
        enable_deferred_sample: false
        deferred_sample_error: false
        deferred_sample_slow_duration: 500
        disable_parent_sampling: false
        resources:
          tenant.id: default
      metrics:
        enabled: false
        client_histogram_buckets: [0.005, 0.01, 0.1, 0.5, 1, 5]
        server_histogram_buckets: [0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 5]
        codes:
          - code: 100014
            type: success
            description: success_desc
          - code: 100015
            type: timeout
            description: timeout_desc
          - code: 100016
            type: exception
            description: exception_desc
            service: ""
            method: ""
      logs:
        enabled: true
        level: info
        enable_sampler: true
        enable_sampler_error: true
        resources:
          tenant.id: default
```

The description of the configuration options is as follows.

| Parameter | Type | Required Configuration | Description |
| ------ | ------ | ------ | ------ |
| **addr** | string | Yes | OpenTelemetry backend service address, receiving data in OpenTelemetry protocol |
| protocol | string | No, default value is "http" | Communication protocol of the backend service, currently supporting "http" and "grpc" protocols |
| selector_name | string | No, default value is "domain" | The method of route selection |
| timeout | int | No, default value is 10000 | Timeout for reporting data, in milliseconds |
| **sampler:fraction** | double | No, default value is 1 | Sampling rate, 1 means full sampling, 0 means no sampling, 0.001 means reporting traces data once for every 1000 calls on average. |
| **traces:disable_trace_body** | bool | No, default value is true | When reporting traces data, whether to upload request and response data, default is off |
| **traces:enable_deferred_sample** | bool | No, default value is false | Whether to enable deferred sampling, additionally reporting erroneous and high latency calls |
| traces:deferred_sample_error | bool | No, default value is false | Whether to sample erroneous calls, with the prerequisite that enable_deferred_sample is set to true |
| traces:deferred_sample_slow_duration | int | No, default value is 500 | Calls with latency higher than this value will be sampled, with the prerequisite that enable_deferred_sample is set to true |
| traces:disable_parent_sampling | bool | No, default value is false | Whether to disable inheriting the upstream sampling flag |
| traces:resources | Mapping | No, default is empty | Resource attributes of the Span |
| **metrics:enabled** | bool | No, default value is false | Whether to enable metrics feature |
| metrics:client_histogram_buckets | Sequences | No, default value is [0.005, 0.01, 0.1, 0.5, 1, 5] | Statistical interval for client-side latency distribution in ModuleReport, measured in seconds. |
| metrics:server_histogram_buckets | Sequences | No, default is [0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 5] | Statistical interval for server-side latency distribution in ModuleReport, measured in seconds. |
| metrics:codes | Mapping | No, default is empty | Error code mapping table, used for customizing error code types |
| **logs:enabled** | bool | No, default value is false | Whether to report remote logs |
| logs:level | string | No, default value is "error" | Log level, only logs with level greater than or equal to level will be reported. Value range: "trace", "debug", "info", "warn", "error", "fatal" |
| logs:enable_sampler | bool | No, default value is false | Whether to report only sampled logs, when enabled, only logs of the current sampled call will be reported |
| logs:enable_sampler_error | bool | No, default value is false | Used in conjunction with enable_sampler, for unsampled calls, if their log level is greater than or equal to error, it will also trigger reporting |
| logs:resources | Mapping | No, default is empty | Resource attributes of the logs |

### Configure the filters

The OpenTelemetry plugin automatically performs the reporting of traces and inter-module metrics data through the filters.

#### Enable ClientFilter

Just add the OpenTelemetry filter in the client configuration of the framework:

```yaml
client:
  filter:
    - opentelemetry
```

#### Enable ServerFilter

Just add the OpenTelemetry filter in the server configuration of the framework:

```yaml
server:
  filter:
    - opentelemetry
```

## Features introduction

**The interfaces of OpenTelemetry plugin mentioned in this section can be imported through the "`trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h`" file.**

### Distributed tracing

#### Collect and report

After configuring the filters, the framework will automatically collect and report traces data during the RPC invocation process. The reported Span information includes the following contents.

1. Common data

    * `Resource`

        In addition to the Resource attributes configured by the user in `traces:resources`, the framework also automatically adds the following attributes.

        | Key | Value |
        |------|------|
        |service.name|app.server|

    * `Attribute`

        | Key | Value |
        |------|------|
        | net.host.ip | Caller IP address |
        | net.host.port | Caller port |
        | net.peer.ip | Callee IP address |
        | net.peer.port | Callee port |
        | trpc.caller_service | Caller service name |
        | trpc.caller_method | Caller method name |
        | trpc.callee_service | Callee service name |
        | trpc.callee_method | Callee method name |
        | trpc.namespace | Namespace |
        | trpc.envname | Env |
        | trpc.dyeing_key | Dyeing data of the framework |
        | trpc.framework_ret | Framework error code (set only when an error occurs during the call) |
        | trpc.func_ret | Interface error code (set only when an error occurs during the call) |
        | trpc.err_msg | Error message (set only when an error occurs during the call) |

    * `Status`

        Set to `StatusCode::kOk` when the call is successful, and set to `StatusCode::kError` when the call fails.

2. Specific attributes

    The Span created by the client filter has a `spanKind` of `SPAN_KIND_CLIENT`, and the `spanName` is the name of the downstream interface being called. The Span created by the server filter has a `spanKind` of `SPAN_KIND_SERVER`, and the `spanName` is the name of the interface currently being called.

3. Request/Response data

    **By default, the framework does not add request and response data to the Span information because converting request and response data to JSON format can affect request latency.** If you confirm that you need to upload this part of the data to help locate the problem, you can set `traces:disable_trace_body` to `false` in the configuration file. After setting, the request and response data will be recorded in two `EVENT`s named `SENT` and `RECEIVE`.

    Note that:
    * Currently, only the data with Protobuf encoding type is supported for reporting.
    * In order to avoid affecting the reporting efficiency when the request/response packet is too large, the framework will truncate the contents of large packets. Users can set the truncation threshold by themselves.

        ```cpp
        /// @brief Sets the maximum allowed length of the request/response data that can be reported
        /// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
        void SetMaxStringLength(uint32_t limit);
        ```

Note that:

* In the proxy mode, it is necessary to invoke the framework's `MakeClientContext` interface to construct the `ClientContext` based on the `ServerContext`. Otherwise, the call relationship between the server and client will be lost, and a complete call chain cannot be formed.

#### Sampling

1. **Default sampling rules**

    The logic is as follows:
    * If the upstream called has been sampled, the current call is also sampled.
    * If the upstream is not sampled, it is sampled according to the `sampler:fraction` sampling rate.

2. Advanced control

    In the actual applications, services usually have a large amount of flow and are not suitable for full reporting. This will cause some critical call chains to not be reported, making it difficult for users to effectively analyze and locate problems. Therefore, we have added some control methods to enable users to selectively report some special calls.

    * **Force sampling**

        Feature: **Users can decide whether to force sampling for the current call based on the specific request information.**

        Usage:

        * Customize a callback function that sets Span startup attributes

            The type of callback function is:

            ```cpp
           /// @brief The type definition of the span's startup attributes setting function. Users can customize startup attributes
            ///        through this callback function.
            /// @param context The context of this call
            /// @param req Pointer to request data, which can be converted to a pointer of a specific request type.
            /// @param [out] attributes Span's startup attributes. It will be passed as the attributes parameter when creating a
            ///                         Span, and can be used in the Sampler. It will ultimately be reflected in the attributes of
            ///                         the reported Span.
            using ServerTraceAttributesFunc = std::function<void(const trpc::ServerContextPtr& context, const void* req,
                                                                 std::unordered_map<std::string, std::string>& attributes)>;
            ```

            Custom the callback function:

            ```cpp
            void TraceAttributesCallback(const trpc::ServerContextPtr& context, const void* req,
                                         std::unordered_map<std::string, std::string>& attributes) {
              if (context->GetFuncName() == "/trpc.test.route.Forward/Route") {
                auto hello_request = static_cast<const ::trpc::test::helloworld::HelloRequest*>(req);
                if (hello_request->msg() == "force") {
                  attributes[::trpc::opentelemetry::kForceSampleKey] = "sample";
                }
              }
            }
            ```

            In the callback function, users can judge the request information based on the context and req parameters. If you decide to sample the call, just need to add an attribute with the key "`::trpc::opentelemetry::kForceSampleKey`" in the attributes.

        * Register callback function

            The registration interface:

            ```cpp
            /// @brief Sets server-side span's startup attributes setting function
            /// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
            void SetServerTraceAttrsFunc(ServerTraceAttributesFunc func);
            ```

            Register the callback function when the service starts:

            ```cpp
            #include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

            class HelloworldServer : public ::trpc::TrpcApp {
             public:
              ...
              int RegisterPlugins() override {
                ::trpc::opentelemetry::Init();
                ::trpc::opentelemetry::SetServerTraceAttrsFunc(TraceAttributesCallback);
                return 0;
              }
            };
            ```

    * **Deferred sampling**

        Feature: **Automatically report calls that have errors and high latency.**

        Usage: Set `traces:enable_deferred_sample` to `true`. Then set `traces:deferred_sample_error` and `traces:deferred_sample_slow_duration` as needed.

        Note:
        * After enabling deferred sampling, the judgment of whether to sample is delayed to the reporting stage, and even Spans that will not be reported in the end will perform actual setting operations. **It will affect the request latency, and users need to weigh its impact before enabling it**.

3. **Complete sampling rules**

    The logic is as follows. It is executed from top to bottom, and if the sampling condition is hit, it will not continue to execute downward.
    * If the startup attributes contain `::trpc::opentelemetry::kForceSampleKey`, it is sampled.
    * If `traces:disable_parent_sampling` is `false` and the `upstream called has been sampled`, it is sampled.
    * Random sampling is performed according to the `sampler:fraction` sampling rate. If it hits, it is sampled.
    * If `deferred sampling` is enabled, it is set to RECORD_ONLY, and whether to sample is delayed to the reporting stage.
    * Otherwise, it is not sampled.

#### Customize span operation

You can retrieve the current Span from the ServerContext using the `::trpc::opentelemetry::GetTracingSpan` interface and then use the native API of [opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp/tree/v1.9.1) to setup the Span.

```cpp
using OpenTelemetryTracingSpanPtr = ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>;

/// @brief Gets the span.
/// @param context server context
/// @return Return the span saved in the context. Note that OpenTelemetryTracingSpanPtr(nullptr) will be returned when
///         there is no valid span in the context.
OpenTelemetryTracingSpanPtr GetTracingSpan(const ServerContextPtr& context);
```

Additionally, we provide convenient interfaces to retrieve the TraceID and SpanID of the current call.

```cpp
/// @brief Gets the trace id.
/// @param context server context
/// @return Return the trace id of the context. Note that empty string will be return when there is no valid span in the
///         context.
std::string GetTraceID(const ServerContextPtr& context);

/// @brief Gets the span id.
/// @param context server context
/// @return Return the span id of the context. Note that empty string will be return when there is no valid span in the
///         context.
std::string GetSpanID(const ServerContextPtr& context);
```

#### Customize the traces transmission method for the corresponding protocol

Different protocols have different methods for transmitting metadata. For example, the `trpc` protocol uses transparent information for passing information, while the `http` protocol can utilize headers for transmission. Therefore, **OpenTelemetry plugin supports configuring different traces transmission methods for different protocols.**.

The plugins currently only support information transmission for the `trpc` and `http` protocols. Other protocols will default to using the same transparent transmission method as the trpc protocol. If the used protocol does not comply with this method, a custom transmission method is required.

Usage:

1. Customize [TextMapCarrier](https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.9.1/api/include/opentelemetry/context/propagation/text_map_propagator.h) for setting and extracting traces data in the opentelemetry-cpp SDK.

    ```cpp
    using TextMapCarrierPtr = std::unique_ptr<::opentelemetry::context::propagation::TextMapCarrier>;
    ```

2. Customize `ClientTextMapCarrierFunc` and `ServerTextMapCarrierFunc` to construct TextMapCarrier based on the Context.

    ```cpp
    using ClientTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ClientContextPtr& context)>;

    using ServerTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ServerContextPtr& context)>;
    ```

3. Register `ClientTextMapCarrierFunc` and `ServerTextMapCarrierFunc` during program startup.

    ```cpp
    /// @brief Sets a client-side TextMapCarrier retrieval function for a specific protocol.
    /// @param protocol_name protocol name
    /// @param carrier_func TextMapCarrier retrieval function
    void SetClientCarrierFunc(const std::string& protocol_name, const ClientTextMapCarrierFunc& carrier_func);

    /// @brief Sets a server-side TextMapCarrier retrieval function for a specific protocol.
    /// @param protocol_name protocol name
    /// @param carrier_func TextMapCarrier retrieval function
    void SetServerCarrierFunc(const std::string& protocol_name, const ServerTextMapCarrierFunc& carrier_func);
    ```

You can refer to the implementation of the trpc and http protocols in [client_filter.cc](./trpc/telemetry/opentelemetry/tracing/client_filter.cc) and [server_filter.cc](./trpc/telemetry/opentelemetry/tracing/server_filter.cc).

### Metrics reporting

The prerequisite for the normal use of the metrics reporting function is to add the `Prometheus compilation option` at compilation and set `metrics: enabled` to `true` in the configuration file.

#### ModuleReport

After configuring the filters, the framework will automatically collect and report inter-module data during RPC calls.

Statistics:

| Metric Name | Metric Type | Description |
| ------ | ------ | ------ |
| rpc_client_started_total | Counter | Total number of calls initiated by the client |
| rpc_client_handled_total | Counter | Total number of calls completed by the client |
| rpc_client_handled_seconds | Histogram | Distribution of client-side call latency (unit: s) |
| rpc_server_started_total | Counter | Total number of requests received by the server |
| rpc_server_handled_total | Counter | Total number of requests processed by the server |
| rpc_server_handled_seconds | Histogram | Distribution of server-side call latency (unit: s) |

All of these statistics include the following statistical labels:

| Key | Value |
| ------ | ------ |
| caller_service | Caller service name |
| caller_method | Caller method name |
| callee_service | Called service name |
| callee_method | Called method name |

`handled_seconds` will have additional statistical labels related to the call result:

| Key | Value |
| ------ | ------ |
| code | Call status code |
| code_type | Status code type, with values of 'success', 'timeout', 'exception' |
| code_desc | Status code description |

#### AttributeReport

In addition to automatically collecting RPC call data, the plugin also defines a set of attribute metrics items internally, allowing users to collect and analyze other required data.

| Metrics Name | Type |
| ------ | ------ |
| opentelemetry_counter_report | Counter |
| opentelemetry_gauge_report | Gauge |
| opentelemetry_summary_report | Summary |
| opentelemetry_histogram_report | Histogram |

The statistical strategies provided by the plugin are as follow.

| Statistical Strategy | Corresponding Metrics Item | Description |
| ------ | ------ | ------ |
| ::trpc::MetricsPolicy::SET | opentelemetry_gauge_report | Set the value, monitor the changes in values. |
| ::trpc::MetricsPolicy::SUM | opentelemetry_counter_report | Calculate the cumulative count of the data. |
| ::trpc::MetricsPolicy::MID | opentelemetry_summary_report | Calculate the median value of the data. |
| ::trpc::MetricsPolicy::QUANTILES | opentelemetry_summary_report | Calculate the specific quantile value of statistical data. |
| ::trpc::MetricsPolicy::HISTOGRAM | opentelemetry_histogram_report | Calculate the interval distribution of statistical data. |

Corresponding to these statistical policies, the plugin provides the following reporting interfaces.

2. Report the data with type `SET`

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with SET type
    /// @param labels metrics labels
    /// @param value the value to set
    /// @return Return 0 for success and non-zero for failure.
    int ReportSetMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

3. Report the data with type `SUM`

    ```cpp
    namespace trpc::opentelemetry {
    
    /// @brief Reports metrics data with SUM type
    /// @param labels metrics labels
    /// @param value the value to increment
    /// @return Return 0 for success and non-zero for failure.
    int ReportSumMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

4. Report the data with type `MID`

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with MID type
    /// @param labels metrics labels
    /// @param value the value to observe
    /// @return Return 0 for success and non-zero for failure.
    int ReportMidMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

5. Report the data with type `QUANTILES`

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with QUANTILES type
    /// @param labels metrics labels
    /// @param quantiles the quantiles used to gather summary statistics
    /// @param value the value to observe
    /// @return Return 0 for success and non-zero for failure.
    int ReportQuantilesMetricsInfo(const std::map<std::string, std::string>& labels, const SummaryQuantiles& quantiles,
                                double value);

    }
    ```

6. Report the data with type `HISTOGRAM`

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with HISTOGRAM type
    /// @param labels metrics labels
    /// @param bucket the bucket used to gather histogram statistics
    /// @param value the value to observe
    /// @return Return 0 for success and non-zero for failure.
    int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, const HistogramBucket& bucket,
                                double value);
    int ReportHistogramMetricsInfo(const std::map<std::string, std::string>& labels, HistogramBucket&& bucket,
                                double value);

    }
    ```

#### Error Code Mapping

The OpenTelemetry plugin's metrics will calculate the success rate, timeout rate, and exception rate of RPC calls based on status codes. The plugin's default status code differentiation policy is:

* 0: Success
* 21, 24, 101, 102: Timeout
* Others: Exception

If the business needs to customize the type of status codes (including framework status codes and business custom status codes), it can be customized through `metrics: codes` in the configuration.

For example, if a user thinks that returning 10001 from the server is a normal situation and should not be counted as an exception, it can be defined as follows:

```yaml
plugins:
  telemetry:
    opentelemetry:
      ...
      metrics:
        ...
        codes:
          - code: 10001
            type: success
            description: exception_desc
            service: ""
            method: ""
```

After the configuration, the plugin will report the case of returning the error code 10001 as a successful call. `service` and `method` can be left unconfigured, and the default logic is to match all services and all methods. If the rule only applies to a specific service and method, it can be specified more specifically.

Note: `type` only supports the three types of "success", "timeout", and "exception", and other types are not effective.

### Logs Collection

The prerequisite for the normal use of the logs reporting function is to add the `log compilation option` at compilation and set `logs:enabled` to `true` in the configuration file.

#### Logs Reporting

The OpenTelemetry log can be printed using the framework's log macro with the `instance` and `context` parameters specified, where `instance` is set to `::trpc::opentelemetry::kOpenTelemetryLoggerName`.

For example:

```cpp
TRPC_LOGGER_FMT_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg: {}", "test");
TRPC_LOGGER_PRT_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg: %s", "test");
TRPC_LOGGER_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg:" << "test");
```

The decision of whether to report logs has three configuration options: `logs:level`, `logs:enable_sampler`, and `logs:enable_sampler_error`. The control logic for each is as follows:

* logs:level: Only logs with levels greater than or equal to `level` will be reported.
* logs:enable_sampler: If `true`, only logs that hit the sampling will be reported, and logs that don't hit the sampling will not be reported. If `false`, all logs will be reported. (Sampling hit means that the call chain of this call is sampled)
* logs:enable_sampler_error: Only effective when `enable_sampler` is `true`. The effect is that even if the sampling is not hit, if the level of the logged message is greater than or equal to `error`, the error log will also be reported.
