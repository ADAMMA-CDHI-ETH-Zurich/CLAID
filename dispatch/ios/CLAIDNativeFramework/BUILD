package(default_visibility = ["//visibility:public"])


load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load("@build_bazel_rules_swift//swift:swift.bzl", "swift_library")
load("@build_bazel_rules_apple//apple:apple.bzl", "apple_xcframework")

###############################
# 1. Build the Native (C/C++) Code
###############################


# objc_library(
#     name = "bridging_header_lib",
#     srcs = [
#         "claid_bridging_header.h",
#     ],
#     copts = [
#         # Tell Swift to use this bridging header
#         "-import-objc-header",
#         "dispatch/ios/claid_swift_package/claid_bridging_header.h",
#     ],
#     deps = [
#         "//dispatch/core:capi",
#     ]
#     # or textual_hdrs if you prefer
# )

# # swift_library(
# #     name = "claid_swift_package",
# #     srcs = glob(["**/*.swift"]),
# #     copts = [
# #         # Tell Swift to use this bridging header
# #         "-import-objc-header",
# #         "dispatch/ios/claid_swift_package/claid_bridging_header.h",
# #     ],
# #     deps = [
# #         ":bridging_header_lib",   # <-- Now recognized by swift_library
# #         "//dispatch/core:capi",
# #     ],
# # )


# objc_library(
#     name = "bridging_header_lib",
#     srcs = [
#         "claid_bridging_header.h",
#     ],
#     # or textual_hdrs if you prefer
# )

# swift_library(
#     name = "claid_swift_package",
#     srcs = glob(["**/*.swift"]),
#     copts = [
#         # Tell Swift to use this bridging header
#         "-import-objc-header",
#         "dispatch/ios/native_claid_xcframework/claid_bridging_header.h",
#     ],
#     deps = [
#         "//dispatch/core:capi",
#     ],
#     module_name="claid_native_xcframework"
# )

###############################
# 2. Build the XCFramework
###############################

objc_library(
    name = "objc_library",
    srcs = [
        "capi_wrapper.h",
    ],
    hdrs = [
        "capi_wrapper.h",
    ],
    
    deps = [
        "//dispatch/core:capi",
    ],
    module_name = "claid_native_xcframework"
)

swift_library(
    name = "swift_library",
    srcs = [
        "Sources/Dummy.swift"
    ],
  
    deps = [
        ":objc_library",
    ],
    module_name = "claid_native_xcframework"
)

apple_xcframework(
    name = "claid_native_xcframework",
    # You can pass multiple frameworks and/or libraries for different platforms,
    # but here's a single iOS static framework example:
    minimum_os_versions = { "ios": "15.0", "tvos": "15.0", },
    infoplists = [":Info.plist"],
    bundle_id = "ch.claid.bundle",
    ios = {
        "simulator": ["arm64"],
        "device": ["arm64"],
    },
    deps = [":objc_library"],   # Link against your cc_library
    public_hdrs = ["Sources/include/capi_wrapper.h"]
    
    # ios = ["arm64"]
    
    # If you’re also building watchOS, tvOS, or macOS variants, list them here:
    # frameworks = [
    #   ":capi_ios_framework",      # iOS
    #   ":capi_watchos_framework",  # watchOS
    #   ":capi_tvos_framework",     # tvOS
    #   ...
    # ]
)

# ###############################
# # 3. Build the Swift Library (with bridging header)
# ###############################

# swift_library(
#     name = "claid_swift_package",
#     srcs = glob(["**/*.swift"]),
#     deps = [
#         "//dispatch/core:capi",
#     ],
#     # If your Bazel version or rules_swift is older and doesn't recognize
#     # `bridging_header`, you can fall back to:
#     copts = [
#         "-import-objc-header", "$(location claid_bridging_header.h)",
#     ],
#     # and then put the bridging header in `data` or `hdrs`.
# )

###############################
# 4. Generate a Package.swift for SwiftPM
###############################
