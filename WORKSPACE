workspace(
    name = "com_github_trpc_ecosystem_cpp_telemetry_opentelemetry",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "trpc_cpp",
    remote = "https://github.com/trpc-group/trpc-cpp.git",
    branch = "main",
)

load("@trpc_cpp//trpc:workspace.bzl", "trpc_workspace")
trpc_workspace()

load("//trpc:workspace.bzl", "telemetry_opentelemetry_workspace")
telemetry_opentelemetry_workspace()
