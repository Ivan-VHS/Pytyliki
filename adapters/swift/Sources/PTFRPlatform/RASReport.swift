import Foundation

public struct RASReport: Decodable, Sendable {
    public let schema: String
    public let provider: String
    public let status: String
    public let readOnly: Bool
    public let hardwareWrites: Bool?
    public let workerAttribution: String?
    public let events: [RASEvent]
    public let droppedRecords: UInt32?
    public let mayHaveMore: Bool?
}

public struct RASEvent: Decodable, Sendable {
    public let recordId: UInt64
    public let timestampNs: UInt64
    public let clockDomain: String
    public let source: String
    public let eventId: UInt32
    public let level: UInt32
    public let severity: String
    public let countDelta: UInt64
    public let controllerId: UInt32
    public let flags: UInt32
    public let attribution: String
}

public struct RASSummary: Equatable, Sendable {
    public let eventCount: Int
    public let corrected: UInt64
    public let uncorrected: UInt64
    public let baselineRecords: Int
    public let droppedRecords: UInt32
}

public enum RASReportError: Error, Equatable {
    case invalidSchema
    case invalidStatus
    case hardwareMutationClaimed
    case malformedEvent(UInt64)
}

public enum RASReportValidator {
    private static let baselineFlag: UInt32 = 1 << 2

    public static func decode(_ data: Data) throws -> RASReport {
        let decoder = JSONDecoder()
        decoder.keyDecodingStrategy = .convertFromSnakeCase
        let report = try decoder.decode(RASReport.self, from: data)
        try validate(report)
        return report
    }

    public static func validate(_ report: RASReport) throws {
        guard report.schema == "pytyliki.ras-provider-report.v1",
              !report.provider.isEmpty,
              report.provider.count <= 160,
              report.readOnly,
              report.events.count <= 32 else {
            throw RASReportError.invalidSchema
        }
        guard report.status == "ok" || report.status == "unsupported" else {
            throw RASReportError.invalidStatus
        }
        if report.hardwareWrites == true {
            throw RASReportError.hardwareMutationClaimed
        }
        if report.workerAttribution != nil &&
            !["none", "not_inferred"].contains(report.workerAttribution!) {
            throw RASReportError.invalidSchema
        }
        if report.status == "unsupported" && !report.events.isEmpty {
            throw RASReportError.invalidStatus
        }

        for event in report.events {
            guard event.recordId > 0,
                  event.eventId > 0,
                  event.countDelta > 0,
                  event.attribution == "system-wide",
                  ["unknown", "corrected", "uncorrected"].contains(event.severity),
                  ["windows-whea", "linux-edac"].contains(event.source),
                  ["unix-realtime", "process-monotonic"].contains(event.clockDomain) else {
                throw RASReportError.malformedEvent(event.recordId)
            }
            if event.source == "linux-edac" &&
                (event.clockDomain != "process-monotonic" ||
                 !["corrected", "uncorrected"].contains(event.severity)) {
                throw RASReportError.malformedEvent(event.recordId)
            }
            if event.source == "windows-whea" && event.clockDomain != "unix-realtime" {
                throw RASReportError.malformedEvent(event.recordId)
            }
        }
    }

    public static func summarize(_ report: RASReport) throws -> RASSummary {
        try validate(report)
        var corrected: UInt64 = 0
        var uncorrected: UInt64 = 0
        var baselineRecords = 0

        for event in report.events {
            if (event.flags & baselineFlag) != 0 {
                baselineRecords += 1
                continue
            }
            switch event.severity {
            case "corrected":
                let next = corrected.addingReportingOverflow(event.countDelta)
                corrected = next.overflow ? UInt64.max : next.partialValue
            case "uncorrected":
                let next = uncorrected.addingReportingOverflow(event.countDelta)
                uncorrected = next.overflow ? UInt64.max : next.partialValue
            default:
                break
            }
        }

        return RASSummary(
            eventCount: report.events.count,
            corrected: corrected,
            uncorrected: uncorrected,
            baselineRecords: baselineRecords,
            droppedRecords: report.droppedRecords ?? 0
        )
    }
}
