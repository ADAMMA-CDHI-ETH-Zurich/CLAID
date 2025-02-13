load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("@rules_cc//cc:defs.bzl", "cc_library")


# iOS Application
ios_application(
    name = "CLAIDSwiftTestApp",
    bundle_id = "com.example.myapp",
    families = ["iphone", "ipad"],
    infoplists = ["Info.plist"],
    minimum_os_version = "14.0",
    deps = ["//dispatch/ios/claid_swift_package:claid_swift_package"],
)