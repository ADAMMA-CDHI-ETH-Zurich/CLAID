package(default_visibility = ["//visibility:public"])

load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("@rules_cc//cc:defs.bzl", "cc_library")


objc_library(
    name = "bridging_header_lib",
    srcs = [
        "claid_bridging_header.h",
    ],
    # or textual_hdrs if you prefer
)

swift_library(
    name = "claid_swift_package",
    srcs = glob(["**/*.swift"]),
    copts = [
        # Tell Swift to use this bridging header
        "-import-objc-header",
        "dispatch/ios/claid_swift_package/claid_bridging_header.h",
    ],
    deps = [
        ":bridging_header_lib",   # <-- Now recognized by swift_library
        "//dispatch/core:capi",
    ],
)