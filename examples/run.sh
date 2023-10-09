bazel build //examples/... --define trpc_include_prometheus=true --copt="-DENABLE_LOGS_PREVIEW"

echo "begin"
./bazel-bin/examples/server/helloworld_server --config=examples/server/trpc_cpp_fiber.yaml &
sleep 1
./bazel-bin/examples/proxy/forward_server --config=examples/proxy/trpc_cpp_fiber.yaml &
sleep 1
./bazel-bin/examples/client/client --config=examples/client/trpc_cpp_fiber.yaml --msg=force
# wait for the OpenTelemetry data to report
sleep 10
killall helloworld_server
if [ $? -ne 0 ]; then
  echo "helloworld_server exit error"
  exit -1
fi

killall forward_server
if [ $? -ne 0 ]; then
  echo "forward_server exit error"
  exit -1
fi
