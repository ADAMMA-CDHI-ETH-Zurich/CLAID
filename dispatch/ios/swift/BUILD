load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("@rules_cc//cc:defs.bzl", "cc_library")

# Swift Library
swift_library(
    name = "AppLib",
    srcs = glob(["swift/**/*.swift"]),
    deps = ["//dispatch/core:capi"],
)

# iOS Application
ios_application(
    name = "MyApp",
    bundle_id = "com.example.myapp",
    families = ["iphone", "ipad"],
    infoplists = ["Info.plist"],
    minimum_os_version = "14.0",
    deps = [":AppLib"],
)