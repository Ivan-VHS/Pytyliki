import Foundation
import Darwin
import PlatformEvidence

guard CommandLine.arguments.count == 2 else {
    fputs("Usage: pytyliki-evidence evidence.jsonl\n", stderr)
    exit(2)
}

do {
    let events = try EvidenceReader.loadJSONL(from: URL(fileURLWithPath: CommandLine.arguments[1]))
    let summary = try EvidenceReader.summarize(events)
    print("PTFR evidence records: \(summary.total)")
    print("Providers: \(summary.providers)")
    print("Kinds: \(summary.kinds)")
    print("Corrected counter total: \(summary.correctedCounterTotal)")
    print("Uncorrected counter total: \(summary.uncorrectedCounterTotal)")
} catch {
    fputs("Evidence load failed: \(error)\n", stderr)
    exit(1)
}
