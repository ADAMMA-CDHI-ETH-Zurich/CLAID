// swift-tools-version: 6.0
// The swift-tools-version declares the minimum version of Swift required to build this package.

import PackageDescription

let package = Package(
    name: "CLAID",
    platforms: [
        .iOS(.v18), // or whichever platforms you support
        .macOS(.v15)
    ],
    products: [
        // Products define the executables and libraries a package produces, making them visible to other packages.
        .library(
            name: "CLAID",
            targets: ["CLAID"]),
    ],
    dependencies: [
         .package(url: "https://github.com/grpc/grpc-swift.git", from: "2.0.0"),
         .package(url: "https://github.com/grpc/grpc-swift-nio-transport.git", from: "1.0.0"),
         .package(url: "https://github.com/grpc/grpc-swift-protobuf.git", from: "1.0.0"),
     ],
    targets: [
        // Targets are the basic building blocks of a package, defining a module or a test suite.
        // Targets can depend on other targets in this package and products from dependencies.
        .target(
            name: "CLAID",
            dependencies: [
                "CLAIDNative",
                .product(name: "GRPCCore", package: "grpc-swift"),
                .product(name: "GRPCNIOTransportHTTP2", package: "grpc-swift-nio-transport"),
                .product(name: "GRPCProtobuf", package: "grpc-swift-protobuf"),
            ],
            plugins: [
                   .plugin(name: "GRPCProtobufGenerator", package: "grpc-swift-protobuf")
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
            path: "../../../bazel-bin/dispatch/ios/CLAIDNativeFramework/claid_native_xcframework.xcframework.zip"
        ),
        .testTarget(
            name: "CLAIDTests",
            dependencies: ["CLAID"]
        ),
    ]
)
