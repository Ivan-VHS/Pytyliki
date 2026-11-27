import Foundation
import XCTest
@testable import PTFRPlatform

final class RASReportTests: XCTestCase {
    func testAcceptsReadOnlyLinuxEDACDelta() throws {
        let json = """
        {
          "schema":"pytyliki.ras-provider-report.v1",
          "provider":"Linux EDAC sysfs counters",
          "status":"ok",
          "read_only":true,
          "hardware_writes":false,
          "worker_attribution":"not_inferred",
          "events":[{
            "recordId":1,"timestampNs":500,"clockDomain":"process-monotonic",
            "source":"linux-edac","eventId":1,"level":0,"severity":"corrected",
            "countDelta":4,"controllerId":0,"flags":2,"attribution":"system-wide"
          }],
          "droppedRecords":0,
          "mayHaveMore":false
        }
        """
        let report = try RASReportValidator.decode(Data(json.utf8))
        let summary = try RASReportValidator.summarize(report)
        XCTAssertEqual(summary.eventCount, 1)
        XCTAssertEqual(summary.corrected, 4)
        XCTAssertEqual(summary.uncorrected, 0)
    }

    func testBaselineCountersAreNotCountedAsFreshFaults() throws {
        let json = """
        {
          "schema":"pytyliki.ras-provider-report.v1",
          "provider":"Linux EDAC",
          "status":"ok","read_only":true,
          "events":[{
            "recordId":2,"timestampNs":500,"clockDomain":"process-monotonic",
            "source":"linux-edac","eventId":1,"level":0,"severity":"corrected",
            "countDelta":12,"controllerId":0,"flags":6,"attribution":"system-wide"
          }]
        }
        """
        let report = try RASReportValidator.decode(Data(json.utf8))
        let summary = try RASReportValidator.summarize(report)
        XCTAssertEqual(summary.corrected, 0)
        XCTAssertEqual(summary.baselineRecords, 1)
    }

    func testRejectsInventedWorkerAttribution() throws {
        let json = """
        {
          "schema":"pytyliki.ras-provider-report.v1",
          "provider":"Windows WHEA",
          "status":"ok","read_only":true,
          "events":[{
            "recordId":3,"timestampNs":500,"clockDomain":"unix-realtime",
            "source":"windows-whea","eventId":17,"level":2,"severity":"unknown",
            "countDelta":1,"controllerId":0,"flags":2,"attribution":"worker-7"
          }]
        }
        """
        XCTAssertThrowsError(try RASReportValidator.decode(Data(json.utf8))) { error in
            XCTAssertEqual(error as? RASReportError, .malformedEvent(3))
        }
    }
}
