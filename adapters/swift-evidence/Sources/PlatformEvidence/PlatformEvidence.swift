import Foundation

public struct EvidenceEvent: Decodable, Sendable {
    public let schema: String
    public let sequence: String
    public let timestamp_epoch_ns: String
    public let provider: String
    public let event_code: UInt32
    public let event_kind: String
    public let severity: String
    public let cpu_id: UInt32?
    public let source_record_id: String
    public let counter_total: String
    public let source_identity: String
    public let payload_sha256: String
    public let event_sha256: String
    public let chain_sha256: String
}

public struct EvidenceSummary: Sendable {
    public let total: Int
    public let providers: [String: Int]
    public let kinds: [String: Int]
    public let correctedCounterTotal: UInt64
    public let uncorrectedCounterTotal: UInt64
}

public enum EvidenceReaderError: Error, Equatable {
    case malformedLine(Int)
    case unsupportedSchema(Int)
    case invalidSequence(Int)
    case invalidDigest(Int)
    case unsafeIdentity(Int)
    case invalidCounter(Int)
}

public enum EvidenceReader {
    public static func loadJSONL(from url: URL) throws -> [EvidenceEvent] {
        let text = try String(contentsOf: url, encoding: .utf8)
        return try decodeJSONL(text)
    }

    public static func decodeJSONL(_ text: String) throws -> [EvidenceEvent] {
        var events: [EvidenceEvent] = []
        let lines = text.split(whereSeparator: \.isNewline)
        for (index, line) in lines.enumerated() {
            let lineNumber = index + 1
            guard let data = String(line).data(using: .utf8),
                  let event = try? JSONDecoder().decode(EvidenceEvent.self, from: data) else {
                throw EvidenceReaderError.malformedLine(lineNumber)
            }
            guard event.schema == "pytyliki.platform-evidence.v1",
                  event.provider == "windows-whea" || event.provider == "linux-edac" else {
                throw EvidenceReaderError.unsupportedSchema(lineNumber)
            }
            guard event.sequence == String(lineNumber),
                  UInt64(event.timestamp_epoch_ns) != nil,
                  UInt64(event.source_record_id) != nil,
                  UInt64(event.counter_total) != nil else {
                throw EvidenceReaderError.invalidSequence(lineNumber)
            }
            guard [event.payload_sha256, event.event_sha256, event.chain_sha256].allSatisfy({
                $0.range(of: "^[0-9a-f]{64}$", options: .regularExpression) != nil
            }) else {
                throw EvidenceReaderError.invalidDigest(lineNumber)
            }
            guard !event.source_identity.isEmpty,
                  event.source_identity.range(of: "^[A-Za-z0-9._/-]+$", options: .regularExpression) != nil,
                  !event.source_identity.contains("..") else {
                throw EvidenceReaderError.unsafeIdentity(lineNumber)
            }
            events.append(event)
        }
        return events
    }

    public static func summarize(_ events: [EvidenceEvent]) throws -> EvidenceSummary {
        var providers: [String: Int] = [:]
        var kinds: [String: Int] = [:]
        var corrected: UInt64 = 0
        var uncorrected: UInt64 = 0

        for event in events {
            providers[event.provider, default: 0] += 1
            kinds[event.event_kind, default: 0] += 1
            guard let total = UInt64(event.counter_total) else {
                throw EvidenceReaderError.invalidCounter(0)
            }
            if event.event_code == 1 {
                let sum = corrected.addingReportingOverflow(total)
                guard !sum.overflow else { throw EvidenceReaderError.invalidCounter(0) }
                corrected = sum.partialValue
            } else if event.event_code == 2 {
                let sum = uncorrected.addingReportingOverflow(total)
                guard !sum.overflow else { throw EvidenceReaderError.invalidCounter(0) }
                uncorrected = sum.partialValue
            }
        }
        return EvidenceSummary(
            total: events.count,
            providers: providers,
            kinds: kinds,
            correctedCounterTotal: corrected,
            uncorrectedCounterTotal: uncorrected
        )
    }
}
