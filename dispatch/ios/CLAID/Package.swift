// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "CLAID",
    platforms: [
        .iOS(.v15), // or whichever platforms you support
        .macOS(.v11)
    ],
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "CLAID",
            targets: ["CLAID"]),
    ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "CLAID",
            dependencies: [
                "CLAIDNative"
            ]
        ),
        // Trick to generate binding headers:
        // create a regular target which depends on the xcframework as binary target.
        .target(
            name: "CLAIDNative",
            dependencies: ["native_xcframework"],
            path: "Sources/CLAIDNative"),
        .binaryTarget(
            name: "native_xcframework",
            path: "../../../bazel-bin/dispatch/ios/native_claid_xcframework/native_xcframework.xcframework.zip"
        ),
        .testTarget(
            name: "CLAIDTests",
            dependencies: ["CLAID"]
        ),
    ]
)
