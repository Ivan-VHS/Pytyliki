import XCTest
@testable import PlatformEvidence

final class PlatformEvidenceTests: XCTestCase {
    private func fixture(sourceIdentity: String = "mc/mc0/ce_count") throws -> String {
        let payload = String(repeating: "a", count: 64)
        let event = String(repeating: "b", count: 64)
        let chain = String(repeating: "c", count: 64)
        let object: [String: Any] = [
            "schema": "pytyliki.platform-evidence.v1",
            "sequence": "1",
            "timestamp_epoch_ns": "1700000000000000000",
            "provider": "linux-edac",
            "event_code": 1,
            "event_kind": "corrected",
            "severity": "warning",
            "cpu_id": NSNull(),
            "source_record_id": "0",
            "counter_total": "7",
            "source_identity": sourceIdentity,
            "payload_sha256": payload,
            "event_sha256": event,
            "chain_sha256": chain
        ]
        let data = try JSONSerialization.data(withJSONObject: object, options: [.sortedKeys])
        return String(decoding: data, as: UTF8.self)
    }

    func testReadsVersionedEvidenceAndSummarizesCounters() throws {
        let records = try EvidenceReader.decodeJSONL(fixture())
        let summary = try EvidenceReader.summarize(records)
        XCTAssertEqual(summary.total, 1)
        XCTAssertEqual(summary.providers["linux-edac"], 1)
        XCTAssertEqual(summary.correctedCounterTotal, 7)
    }

    func testRejectsUnknownSchema() throws {
        let source = try fixture().replacingOccurrences(
            of: "pytyliki.platform-evidence.v1",
            with: "wrong.schema"
        )
        XCTAssertThrowsError(try EvidenceReader.decodeJSONL(source))
    }

    func testRejectsPathTraversalIdentity() throws {
        let source = try fixture(sourceIdentity: "../../etc/passwd")
        XCTAssertThrowsError(try EvidenceReader.decodeJSONL(source))
    }
}
