// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "PytylikiPlatformEvidence",
    platforms: [.macOS(.v13)],
    products: [
        .library(name: "PlatformEvidence", targets: ["PlatformEvidence"]),
        .executable(name: "pytyliki-evidence", targets: ["EvidenceCLI"])
    ],
    targets: [
        .target(name: "PlatformEvidence"),
        .executableTarget(name: "EvidenceCLI", dependencies: ["PlatformEvidence"]),
        .testTarget(name: "PlatformEvidenceTests", dependencies: ["PlatformEvidence"])
    ]
)
