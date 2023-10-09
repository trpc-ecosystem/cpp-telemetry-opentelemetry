# OpenTelemetry telemetry demo

The server invocation relationship is as follows:

client <---> proxy <---> server

Since the usage of the OpenTelemetry plugin is independent of the runtime type, this demo will only use the fiber mode for demonstration purposes.


## Usage

We can use the following command to view the directory tree.
```shell
$ tree examples/
examples/
├── client
│   ├── BUILD
│   ├── client.cc
│   └── trpc_cpp_fiber.yaml
├── proxy
│   ├── BUILD
│   ├── forward.proto
│   ├── forward_server.cc
│   ├── forward_service.cc
│   ├── forward_service.h
│   └── trpc_cpp_fiber.yaml
├── README.md
├── run.sh
└── server
    ├── BUILD
    ├── greeter_service.cc
    ├── greeter_service.h
    ├── helloworld.proto
    ├── helloworld_server.cc
    └── trpc_cpp_fiber.yaml
```

* Compilation

We can run the following command to compile the demo.

```shell
$ bazel build //examples/... --define trpc_include_prometheus=true --copt="-DENABLE_LOGS_PREVIEW"
```

* Run the server/proxy program

We can run the following command to start the server and proxy program.

```shell
$ ./bazel-bin/examples/server/helloworld_server --config=examples/server/trpc_cpp_fiber.yaml
```

```shell
$ ./bazel-bin/examples/proxy/forward_server --config=examples/proxy/trpc_cpp_fiber.yaml
```

* Run the client program

We can run the following command to start the client program.

```shell
$ ./bazel-bin/examples/client/client --config=examples/client/trpc_cpp_fiber.yaml
```

* View the tracing data

You need to first start an OpenTelemetry Collector and specify the "opentelemetry:addr" configuration item as the receiving address of the Collector. Then, you can view the traces and logs data in the background of the Collector, while the metrics data is collected and reported using the pull method, which can be viewed by accessing http://admin_ip:admin_port/metrics.
