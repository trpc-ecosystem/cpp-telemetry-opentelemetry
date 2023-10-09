[English](./README.md)

[TOC]

# 前言

[OpenTelemetry](https://opentelemetry.io/)是业界流行的一个可观测性框架和工具集，用于创建和管理调用链、监控指标和日志等遥测数据。我们提供了一个OpenTelemetry遥测插件，让用户在tRPC-Cpp中可以方便地使用OpenTelemetry来采集和上报遥测数据。

在OpenTelemetry遥测插件中，上报的调用链和日志数据采用OpenTelemetry协议，而监控数据的上报利用了框架的Prometheus能力，采用[Prometheus](https://prometheus.io/)格式。

# 使用说明

详细的使用例子可以参考：[OpenTelemetry examples](./examples/)。

## 接入方式

要在项目中使用OpenTelemetry遥测插件，需要按照以下步骤进行接入。

### 引入依赖

#### Bazel

1. 引入仓库

    在项目的`WORKSPACE`文件中，引入`cpp-telemetry-opentelemetry`仓库及其依赖：
    ```
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

2. 引入插件

    在需要用到OpenTelemetry的目标中引入“`trpc/telemetry/opentelemetry:opentelemetry_telemetry_api`”依赖。例如：
    ```
    cc_binary(
        name = "helloworld_server",
        srcs = ["helloworld_server.cc"],
        deps = [
            "@cpp-telemetry-opentelemetry//trpc/telemetry/opentelemetry:opentelemetry_telemetry_api",
            ...
        ],
    )
    ```

3. 编译选项

    * 由于监控功能用到了框架的Prometheus能力，所以要使用插件的监控功能，需要加上“`trpc_include_prometheus`”编译选项。例如在.bazelrc中加上：
        ```
        build --define trpc_include_prometheus=true
        ```

    * 由于当前引入的[opentelemetry-cpp v1.9.1版本](https://github.com/open-telemetry/opentelemetry-cpp/tree/v1.9.1)中，日志功能仍然处于预览状态，所以要使用插件的日志功能，需要加上“`ENABLE_LOGS_PREVIEW`”编译宏。例如在.bazelrc中加上：
        ```
        build --copt="-DENABLE_LOGS_PREVIEW"
        ```

#### CMake

暂不支持。

### 注册

OpenTelemetry插件提供了插件和拦截器注册的接口`::trpc::opentelemetry::Init`，用户需要在框架启动前调用该接口进行初始化。

1. 对于服务端场景，用户需要在服务启动的`TrpcApp::RegisterPlugins`函数中调用：

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

2. 对于纯客户端场景，需要在启动框架配置初始化后，框架其他模块启动前调用：

    ```cpp
    #include "trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h"

    int main(int argc, char* argv[]) {
      ParseClientConfig(argc, argv);

      ::trpc::opentelemetry::Init();

      return ::trpc::RunInTrpcRuntime([]() { return Run(); });
    }
    ```

### 配置插件

必须在框架配置文件中加上OpenTelemetry插件的配置。
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

配置项说明：

| 参数 | 类型 | 是否必须配置 | 说明 |
| ------ | ------ | ------ | ------ |
| **addr** | string | 是 | OpenTelemetry后端服务地址，接收OpenTelemetry协议的数据 |
| protocol | string | 否，默认为"http" | 后端服务的通信协议，当前支持"http"和"grpc"协议 |
| selector_name | string | 否，默认为"domain" | 路由选择的方式 |
| timeout | int | 否，默认为10000 | 上报数据的超时时间，单位为ms |
| **sampler:fraction** | double | 否，默认为1 | 采样率，配置为1表示全采样，配置为0表示不采样，设置为0.001表示平均每1000次调用上报一次调用链数据。 |
| **traces:disable_trace_body** | bool | 否，默认为true | 上报调用链信息时，是否上传请求和响应数据，默认关闭 |
| **traces:enable_deferred_sample** | bool | 否，默认为false | 是否开启延迟采样, 额外上报出错的/高耗时的调用 |
| traces:deferred_sample_error | bool | 否，默认为false | 是否采样出错的调用，前提条件是enable_deferred_sample设置为true |
| traces:deferred_sample_slow_duration | int | 否，默认为500 | 耗时高于该值的调用将会被采样，前提条件是enable_deferred_sample设置为true |
| traces:disable_parent_sampling | bool | 否，默认为false | 是否关闭继承上游的采样标志 |
| traces:resources | 映射（Mapping） | 否，默认为空 | Span的Resource标签 |
| **metrics:enabled** | bool | 否，默认为false | 是否启用监控功能 |
| metrics:client_histogram_buckets | 序列（Sequences） | 否，默认为[0.005, 0.01, 0.1, 0.5, 1, 5] | 客户端模调监控耗时分布的统计区间，单位为s |
| metrics:server_histogram_buckets | 序列（Sequences） | 否，默认为[0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1, 5] | 服务端模调监控耗时分布的统计区间，单位为s |
| metrics:codes | 映射（Mapping） | 否，默认为空 | 错误码映射表，用于自定义错误码的类型 |
| **logs:enabled** | bool | 否，默认为false | 是否上报远程日志 |
| logs:level | string | 否，默认为"error" | 日志级别，只有级别大于等于level的日志才会上报。取值范围："trace"，"debug"，"info"，"warn"，"error"，"fatal" |
| logs:enable_sampler | bool | 否，默认为false | 是否只上报采样日志, 启用后只有当前调用命中采样时才会上报 |
| logs:enable_sampler_error | bool | 否，默认为false | 与enable_sampler配合使用，对于未采样的调用，若其日志级别大于等于error，也会触发上报 |
| logs:resources | 映射（Mapping） | 否，默认为空 | 日志的Resource标签 |

### 配置拦截器

OpenTelemetry插件的调用链和模调数据上报通过拦截器来自动执行。

#### 启用客户端拦截器

只需要在框架的客户端配置中加上OpenTelemetry拦截器即可：
```yaml
client:
  filter:
    - opentelemetry
```

#### 启用服务端拦截器

只需要在框架的服务端配置中加上OpenTelemetry拦截器即可：
```yaml
server:
  filter:
    - opentelemetry
```

## 功能介绍

**本节提到的OpenTelemetry插件接口均可以通过”`trpc/telemetry/opentelemetry/opentelemetry_telemetry_api.h`“文件来引入。**

### 链路追踪

#### 采集和上报

配置拦截器后，框架会自动在RPC调用过程中采集和上报调用链数据。上报的Span信息包含以下内容：

1. 公有数据

    * `Resource`

        除了用户在`traces:resources`中配置的Resource标签外，框架还会自动加上：

        | Key | Value |
        |------|------|
        |service.name|app.server|

    * `Attribute`

        | Key | Value |
        |------|------|
        | net.host.ip | 主调ip地址 |
        | net.host.port | 主调端口 |
        | net.peer.ip | 被调ip地址 |
        | net.peer.port | 被调端口 |
        | trpc.caller_service | 主调服务名 |
        | trpc.caller_method | 主调方法名 |
        | trpc.callee_service | 被调服务名 |
        | trpc.callee_method | 被调方法名 |
        | trpc.namespace | namespace |
        | trpc.envname | env |
        | trpc.dyeing_key | 框架的染色数据 |
        | trpc.framework_ret | 框架错误码（调用发生错误时才会设置） |
        | trpc.func_ret | 接口错误码（调用发生错误时才会设置） |
        | trpc.err_msg | 错误信息（调用发生错误时才会设置） |

    * `Status`

        调用成功时设置为`StatusCode::kOk`，调用失败时设置为`StatusCode::kError`。

2. 特有属性

    客户端拦截器创建的Span，其`spanKind`为`SPAN_KIND_CLIENT`，`spanName`是调用下游的接口名。而服务端拦截器创建的Span，其`spanKind`为`SPAN_KIND_SERVER`，`spanName`是当前被调用的接口名。

3. 请求/响应数据

    **框架默认不会在Span信息中添加请求数据和响应数据，因为涉及将请求和响应数据转换为json格式的操作，会影响请求的耗时时间。** 若确认需要上传这部分数据辅助定位问题，可以在配置文件中将`traces:disable_trace_body`设置为`false`。设置后请求和响应数据将记录在名字为`SENT`和`RECEIVE`的两个`EVENT`中。

    注意：
    * 当前只支持上报Protobuf编码类型的数据，其他编码类型暂不支持。
    * 为了避免请求/响应包过大的情况下影响上报效率，框架会对大包的内容进行截断。用户可以自行设置截断的阈值：
        ```cpp
        /// @brief Sets the maximum allowed length of the request/response data that can be reported
        /// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
        void SetMaxStringLength(uint32_t limit);
        ```

注意：
* 在中转模式下，`ClientContext`需要调用框架的`MakeClientContext`接口，根据`ServerContext`来构造。否则服务端和客户端之间的调用关系会丢失，无法构成完整的调用链。

#### 采样

1. **默认的采样规则**

    逻辑如下：
    * 若当前调用的上游已采样，则当前调用也采样。
    * 若上游未采样，则按照`sampler:fraction`采样率进行采样。

2. 高级控制

    在实际生产应用中，服务通常会有较大的流量，并不适合采用全量上报。这就会导致一些关键的调用链路没有被上报，无法有效地帮助用户分析定位问题。因此我们增加了一些控制方式，使得用户可以对某些特殊的调用进行针对性地上报。

    * **强制采样**

        特点：**用户可以根据请求的具体信息，来决定是否对当前的调用进行强制采样。**
    
        使用方法：

        * 自定义一个设置Span启动属性的回调函数

            回调函数类型：
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

            自定义回调函数：
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

            在回调函数中，用户可以根据context和req参数对请求信息进行判断。如果确定要对该调用进行采样，只需要在attributes中添加key为“`::trpc::opentelemetry::kForceSampleKey`”的属性即可。

        * 注册回调函数

            注册接口：
            ```cpp
            /// @brief Sets server-side span's startup attributes setting function
            /// @note The interface is not thread-safe, and users should only set it during the framework initialization process.
            void SetServerTraceAttrsFunc(ServerTraceAttributesFunc func);
            ```

            在服务启动时注册该回调函数：
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

    * **延迟采样**

        特点：**自动上报发生错误的和高延迟的调用。**

        使用方法：将`traces:enable_deferred_sample`配置为`true`。然后根据需要对`traces:deferred_sample_error`和`traces:deferred_sample_slow_duration`进行设置。

        注意：
        * 开启延迟采样后，会将是否采样的判断延迟到上报阶段，即使最后不会上报的Span也会执行实际的设置操作。**其会影响请求的耗时，用户在开启前需要衡量其影响**。

3. **完整的采样规则**

    逻辑如下，其从上往下依次执行，命中采样条件则不继续往下执行。
    * 若启动属性中包含`::trpc::opentelemetry::kForceSampleKey`，则采样。
    * 若`traces:disable_parent_sampling`为`false`，并且当前调用的`上游已采样`，则采样。
    * 按照`sampler:fraction`采样率进行随机采样，若命中则采样。
    * 若开启了`延迟采样`，则设置为RECORD_ONLY，将是否采样延迟到上报阶段决定。
    * 否则不采样。

#### 自定义Span操作

可以通过`::trpc::opentelemetry::GetTracingSpan`接口从ServerContext中取出当前调用的Span，然后调用[opentelemetry-cpp](https://github.com/open-telemetry/opentelemetry-cpp/tree/v1.9.1)原生的API对Span进行设置。
```cpp
using OpenTelemetryTracingSpanPtr = ::opentelemetry::nostd::shared_ptr<::opentelemetry::trace::Span>;

/// @brief Gets the span.
/// @param context server context
/// @return Return the span saved in the context. Note that OpenTelemetryTracingSpanPtr(nullptr) will be returned when
///         there is no valid span in the context.
OpenTelemetryTracingSpanPtr GetTracingSpan(const ServerContextPtr& context);
```

另外，我们提供了便捷的接口获取当前调用的TraceID和SpanID。
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

#### 自定义协议的链路信息传递方式

不同的协议传递元数据的方法不同，例如`trpc`协议通过透传信息传递，`http`协议可以通过头部传递。所以**OpenTelemetry插件支持对不同的协议设置不同的链路信息设置和提取方式**。

插件目前只支持了`trpc`协议和`http`协议的信息传递，其他的协议默认会走跟trpc协议一样的透传信息传递方式。若使用的协议不符合这种方式，则需要自定义传递方式。

使用方法：

1. 自定义[TextMapCarrier](https://github.com/open-telemetry/opentelemetry-cpp/blob/v1.9.1/api/include/opentelemetry/context/propagation/text_map_propagator.h)，用于opentelemetry-cpp SDK设置和提取调用链信息。
    ```cpp
    using TextMapCarrierPtr = std::unique_ptr<::opentelemetry::context::propagation::TextMapCarrier>;
    ```

2. 自定义`ClientTextMapCarrierFunc`和`ServerTextMapCarrierFunc`，根据Context构造`TextMapCarrier`。
    ```cpp
    using ClientTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ClientContextPtr& context)>;

    using ServerTextMapCarrierFunc = std::function<TextMapCarrierPtr(const ServerContextPtr& context)>;
    ```

3. 在程序启动时注册`ClientTextMapCarrierFunc`和`ServerTextMapCarrierFunc`。
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

可以参考[client_filter.cc](./trpc/telemetry/opentelemetry/tracing/client_filter.cc)和[server_filter.cc](./trpc/telemetry/opentelemetry/tracing/server_filter.cc)中trpc和http协议的实现。

### 监控上报

**注意监控上报功能正常使用的前提条件是编译时加上`Prometheus的编译选项`，以及配置文件中`metrics:enabled`设置为`true`。**

#### 模调上报

配置拦截器后，框架会自动在RPC调用过程中采集和上报模调数据。

统计项：

| 监控名 | 监控类型 | 说明 |
| ------ | ------ | ------ |
| rpc_client_started_total | Counter | 客户端发起的调用总次数 |
| rpc_client_handled_total | Counter | 客户端完成的调用次数 |
| rpc_client_handled_seconds | Histogram | 客户端调用的耗时分布（单位：s） |
| rpc_server_started_total | Counter | 服务端收到的请求总次数 |
| rpc_server_handled_total | Counter | 服务端处理完成的请求总次数 |
| rpc_server_handled_seconds | Histogram | 服务端处理请求的耗时分布（单位：s） |

这些统计项均包含如下的统计标签：

| Key | Value |
| ------ | ------ |
| caller_service | 主调服务名 |
| caller_method | 主调方法名 |
| callee_service | 被调服务名 |
| callee_method | 被调方法名 |

`handled_seconds`会额外多出调用结果相关的统计标签：

| Key | Value |
| ------ | ------ |
| code | 调用状态码 |
| code_type | 状态码类型，取值范围："success"，"timeout"，"exception" |
| code_desc | 状态码描述 |

#### 属性上报

除了框架自动采集的RPC调用数据外，插件内部还定义了一组属性监控项，用于用户对其他需要的数据进行采集和统计：

| 监控名 | 类型 |
| ------ | ------ |
| opentelemetry_counter_report | Counter |
| opentelemetry_gauge_report | Gauge |
| opentelemetry_summary_report | Summary |
| opentelemetry_histogram_report | Histogram |

插件提供了如下的统计策略：

| 统计策略 | 对应监控项 | 说明 |
| ------ | ------ | ------ |
| ::trpc::MetricsPolicy::SET | opentelemetry_gauge_report | 设置值，监控值的变化 |
| ::trpc::MetricsPolicy::SUM | opentelemetry_counter_report | 计算标签的累积计数 |
| ::trpc::MetricsPolicy::MID | opentelemetry_summary_report | 统计数据的中位数 |
| ::trpc::MetricsPolicy::QUANTILES | opentelemetry_summary_report | 统计数据的具体分位数值 |
| ::trpc::MetricsPolicy::HISTOGRAM | opentelemetry_histogram_report | 统计数据的区间分布 |

对应这些统计策略，插件提供了如下的上报接口：

1. 上报`SET`类型数据

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with SET type
    /// @param labels metrics labels
    /// @param value the value to set
    /// @return Return 0 for success and non-zero for failure.
    int ReportSetMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

2. 上报`SUM`类型数据

    ```cpp
    namespace trpc::opentelemetry {
    
    /// @brief Reports metrics data with SUM type
    /// @param labels metrics labels
    /// @param value the value to increment
    /// @return Return 0 for success and non-zero for failure.
    int ReportSumMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

3. 上报`MID`类型数据

    ```cpp
    namespace trpc::opentelemetry {

    /// @brief Reports metrics data with MID type
    /// @param labels metrics labels
    /// @param value the value to observe
    /// @return Return 0 for success and non-zero for failure.
    int ReportMidMetricsInfo(const std::map<std::string, std::string>& labels, double value);

    }
    ```

4. 上报`QUANTILES`类型数据

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

5. 上报`HISTOGRAM`类型数据

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

#### 错误码映射

OpenTelemetry插件的监控会统计RPC调用的成功率、超时率和异常率，具体的统计方式是根据状态码进行区分。插件默认的状态码区分策略为：

* 0：成功
* 21、24、101、102：超时
* 其他：异常

如果业务需要对状态码（包括框架状态码和业务自定义状态码）的类型进行自定义，则可以通过配置中的`metrics:codes`进行自定义。

例如用户认为服务端返回10001是正常的情况，不应该统计成异常，则可以做如下定义：
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

配置之后，插件就会将返回10001错误码的情况上报为成功的调用。`service`和`method`可以不配置，默认为匹配全部service和全部method，若该规则只适用于某个service某个method，则可以进行更具体地指定。

注意：`type`只支持"success"、"timeout"、"exception"三种，其他类型不生效

### 日志采集

**注意日志上报功能正常使用的前提条件是编译时加上`日志编译选项`，以及配置文件中`logs:enabled`设置为`true`。**

#### 日志上报

可以使用框架带`instance`和`context`参数的日志宏来打印OpenTelemetry日志，其中`instance`指定为`::trpc::opentelemetry::kOpenTelemetryLoggerName`。

例如：
```cpp
TRPC_LOGGER_FMT_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg: {}", "test");
TRPC_LOGGER_PRT_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg: %s", "test");
TRPC_LOGGER_INFO_EX(context, ::trpc::opentelemetry::kOpenTelemetryLoggerName, "msg:" << "test");
```

决定日志是否上报有三个配置选项：`logs:level`、`logs:enable_sampler`、`logs:enable_sampler_error`。各自的控制逻辑如下：
* logs:level：只有级别大于等于level的日志才会上报。
* logs:enable_sampler：若为true，则只有命中采样的日志才会上报，未命中采样的日志不会上报；若为false，则全部日志都会上报。（命中采样是指此次调用的调用链被采样）
* logs:enable_sampler_error：只有enable_sampler为true的情况下才生效。效果是：即使未命中采样，但只要打印的日志其级别大于等于error，则该错误日志也会被上报。
