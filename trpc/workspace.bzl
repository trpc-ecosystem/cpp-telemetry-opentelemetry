"""This module contains some dependency"""

# buildifier: disable=load
load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

def clean_dep(dep):
    return str(Label(dep))

# buildifier: disable=function-docstring-args
def telemetry_opentelemetry_workspace(path_prefix = "", repo_name = "", **kwargs):
    """Build rules for the trpc project

    Note: The main idea is to determine the required version of dependent software during the build process
          by passing in parameters.
    Args:
        path_prefix: Path prefix.
        repo_name: Repository name of the dependency.
        kwargs: Keyword arguments, dictionary type, mainly used to specify the version and sha256 value of
                dependent software, where the key of the keyword is constructed by the `name + version`.
                eg: protobuf_ver,zlib_ver...
    Example:
        trpc_workspace(path_prefix="", repo_name="", protobuf_ver="xxx", protobuf_sha256="xxx", ...)
        Here, `xxx` is the specific specified version. If the version is not specified through the key,
        the default value will be used. eg: protobuf_ver = kwargs.get("protobuf_ver", "3.8.0")
    """

    # com_github_curl_curl
    curl_ver = kwargs.get("curl_ver", "7.81.0")
    curl_path_ver = kwargs.get("curl_path_ver", "7_81_0")
    curl_sha256 = kwargs.get("curl_sha256", "61570dcebdf913c3675c91decd512f9bfe352f257036a86b73dabf2035eefeca")
    curl_urls = [
        "https://github.com/curl/curl/releases/download/curl-{path_ver}/curl-{ver}.zip".format(ver = curl_ver, path_ver = curl_path_ver),
    ]
    http_archive(
        name = "com_github_curl_curl",
        sha256 = curl_sha256,
        strip_prefix = "curl-{ver}".format(ver = curl_ver),
        build_file = "//third_party/com_github_curl_curl:curl.BUILD",
        urls = curl_urls,
        patches = [
            "//third_party/com_github_curl_curl:0001-generate-header-files.patch",
        ],
        patch_args = ["-p1"],
    )

    # io_opentelemetry_cpp
    io_opentelemetry_cpp_ver = kwargs.get("io_opentelemetry_cpp_ver", "1.9.1")
    io_opentelemetry_cpp_sha256 = kwargs.get("io_opentelemetry_cpp_sha256", "668de24f81c8d36d75092ad9dcb02a97cd41473adbe72485ece05e336db48249")
    io_opentelemetry_cpp_urls = [
        "https://github.com/open-telemetry/opentelemetry-cpp/archive/v{ver}.tar.gz".format(ver = io_opentelemetry_cpp_ver),
    ]
    http_archive(
        name = "io_opentelemetry_cpp",
        sha256 = io_opentelemetry_cpp_sha256,
        strip_prefix = "opentelemetry-cpp-{ver}".format(ver = io_opentelemetry_cpp_ver),
        urls = io_opentelemetry_cpp_urls,
        repo_mapping = {
            "@curl": "@com_github_curl_curl",
        },
    )

    # com_github_opentelemetry_proto
    com_github_opentelemetry_proto_ver = kwargs.get("com_github_opentelemetry_proto_ver", "0.19.0")
    com_github_opentelemetry_proto_sha256 = kwargs.get("com_github_opentelemetry_proto_sha256", "464bc2b348e674a1a03142e403cbccb01be8655b6de0f8bfe733ea31fcd421be")
    com_github_opentelemetry_proto_urls = [
        "https://github.com/open-telemetry/opentelemetry-proto/archive/v{ver}.tar.gz".format(ver = com_github_opentelemetry_proto_ver),
    ]
    http_archive(
        name = "com_github_opentelemetry_proto",
        sha256 = com_github_opentelemetry_proto_sha256,
        strip_prefix = "opentelemetry-proto-{ver}".format(ver = com_github_opentelemetry_proto_ver),
        build_file = clean_dep("//third_party/com_github_opentelemetry_proto:opentelemetry_proto.BUILD"),
        urls = com_github_opentelemetry_proto_urls,
    )

    # github_nlohmann_json
    github_nlohmann_json_ver = kwargs.get("github_nlohmann_json_ver", "3.11.2")
    github_nlohmann_json_sha256 = kwargs.get("github_nlohmann_json_sha256", "d69f9deb6a75e2580465c6c4c5111b89c4dc2fa94e3a85fcd2ffcd9a143d9273")
    github_nlohmann_json_urls = [
        "https://github.com/nlohmann/json/archive/refs/tags/v{ver}.tar.gz".format(ver = github_nlohmann_json_ver),
    ]
    http_archive(
        name = "github_nlohmann_json",
        sha256 = github_nlohmann_json_sha256,
        strip_prefix = "json-{ver}".format(ver = github_nlohmann_json_ver),
        build_file = clean_dep("@io_opentelemetry_cpp//bazel:nlohmann_json.BUILD"),
        urls = github_nlohmann_json_urls,
    )
