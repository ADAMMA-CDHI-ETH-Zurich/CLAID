# load ios application rule
load("@build_bazel_rules_apple//apple:ios.bzl", "ios_application")

# load build in objc_library rule 
load("@rules_cc//cc:defs.bzl", "objc_library")

objc_library(
    name = "BazelHelloworldObjC_classes",
    srcs = [
         "BazelHelloworldObjC/AppDelegate.m",
         "BazelHelloworldObjC/SceneDelegate.m",
         "BazelHelloworldObjC/ViewController.m",
         "BazelHelloworldObjC/main.mm",
         "BazelHelloworldObjC/CLAIDWrapper.cpp",
         "BazelHelloworldObjC/TestSenderModule.cpp",
         "BazelHelloworldObjC/TestReceiverModule.cpp"
         
    ],
    hdrs = glob(["BazelHelloworldObjC/*.h", "BazelHelloworldObjC/*.hpp"]),
    data = [
    		"BazelHelloworldObjC/Base.lproj/Main.storyboard",
    		"BazelHelloworldObjC/Base.lproj/LaunchScreen.storyboard",
            "claid_config.json",
    ],
    deps=["//dispatch/core:capi"],
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
    launch_storyboard = "BazelHelloworldObjC/Base.lproj/LaunchScreen.storyboard",
    visibility = ["//visibility:public"],
    deps = [":BazelHelloworldObjC_classes", "//dispatch/core:capi"],
)