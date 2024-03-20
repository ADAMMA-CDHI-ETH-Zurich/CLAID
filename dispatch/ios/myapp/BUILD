# load ios application rule
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")
load(
    "@rules_xcodeproj//xcodeproj:defs.bzl",
    "top_level_target",
    "xcodeproj",
)
# load build in objc_library rule 
load("@rules_cc//cc:defs.bzl", "objc_library")

objc_library(
    name = "BazelHelloworldObjC_classes",
    srcs = glob([
         "BazelHelloworldObjC/**/*.m",
         "BazelHelloworldObjC/**/*.mm",
         "BazelHelloworldObjC/**/*.cpp",
         
    ]),
    hdrs = glob(["BazelHelloworldObjC/**/*.h", "BazelHelloworldObjC/**/*.hpp", "BazelHelloworldObjC/**/*.hh"]),
    data = [
    		"BazelHelloworldObjC/Base.lproj/Main.storyboard",
    		"BazelHelloworldObjC/Base.lproj/LaunchScreen.storyboard",
            "claid_config.json",
    ],
    deps=["//dispatch/core:capi"],
    linkopts = ["-Wl,-framework,CoreMotion"],
    sdk_frameworks = ["CoreLocation"],
    alwayslink = True,
    # copts=["-std=c++14"]
)

# Use ios application rule
ios_application(
    name = "App",
    bundle_id = "com.domain.appBundleID",
    families = [
        "iphone",
        "ipad",
    ],
    minimum_os_version = "13.0",
    infoplists = [":BazelHelloworldObjC/Info.plist"],
    #Must have launch_storyboard, otherwise UI goes funny
    provisioning_profile = "test.mobileprovision",
    launch_storyboard = "BazelHelloworldObjC/Base.lproj/LaunchScreen.storyboard",
    visibility = ["//visibility:public"],
    deps = [":BazelHelloworldObjC_classes", "//dispatch/core:capi"],
)

xcodeproj(
    name = "xcodeproj",
    project_name = "App",
    tags = ["manual"],
    top_level_targets = [
        top_level_target(":App", target_environments = ["device", "simulator"])    ],
)

