#! /bin/bash

bazel build //trpc/... --define trpc_include_prometheus=true --copt="-DENABLE_LOGS_PREVIEW"
