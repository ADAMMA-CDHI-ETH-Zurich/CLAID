package(default_visibility = ["//visibility:public"])


load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

###############################
# 1. Build the Native (C/C++) Code
###############################

cc_library(
    name = "capi_wrapper",
    srcs = [
        "capi_wrapper.cpp",
    ],
    hdrs = [
        "capi_wrapper.h",
    ],
    deps = [
        "//dispatch/core:capi",
    ],
)


###############################
# 2. Build the XCFramework
###############################
apple_xcframework(
    name = "native_xcframework",
    # You can pass multiple frameworks and/or libraries for different platforms,
    # but here's a single iOS static framework example:
    deps = [":capi_wrapper"],   # Link against your cc_library
    minimum_os_versions = { "ios": "13.0", "tvos": "15.0", },
    infoplists = [":Info.plist"],
    bundle_id = "com.domain.appBundleID",
    
    # If you’re also building watchOS, tvOS, or macOS variants, list them here:
    # frameworks = [
    #   ":capi_ios_framework",      # iOS
    #   ":capi_watchos_framework",  # watchOS
    #   ":capi_tvos_framework",     # tvOS
    #   ...
    # ]
)

###############################
# 3. Build the Swift Library (with bridging header)
###############################

swift_library(
    name = "claid_swift_package",
    srcs = glob(["**/*.swift"]),
    deps = [
        "//dispatch/core:capi",
    ],
    # If your Bazel version or rules_swift is older and doesn't recognize
    # `bridging_header`, you can fall back to:
    copts = [
        "-import-objc-header", "$(location claid_bridging_header.h)",
    ],
    # and then put the bridging header in `data` or `hdrs`.
)

###############################
# 4. Generate a Package.swift for SwiftPM
###############################
