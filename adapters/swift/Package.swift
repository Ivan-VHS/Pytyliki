// swift-tools-version: 6.0
import PackageDescription

let package = Package(
    name: "PTFRPlatform",
    platforms: [.macOS(.v13)],
    products: [
        .library(name: "PTFRPlatform", targets: ["PTFRPlatform"])
    ],
    targets: [
        .target(name: "PTFRPlatform"),
        .testTarget(name: "PTFRPlatformTests", dependencies: ["PTFRPlatform"])
    ]
)
