import assert from "node:assert/strict";
import test from "node:test";
import { createHash } from "node:crypto";
import { verifyEvidenceText, verifyLinkText } from "../src/verify.ts";

const hash = (value: string): string =>
  createHash("sha256").update(value, "utf8").digest("hex");

function makeRecord() {
  const payload = hash("mc/mc0/ce_count|1000|1|1|7");
  const canonical = [
    "pytyliki.platform-evidence.v1",
    "1000", "1", "7", "2", "1", "4294967295", "2", "1",
    "mc/mc0/ce_count", payload,
  ].join("|");
  const event = hash(canonical);
  const chain = hash(\`pytyliki.evidence-chain.v1|1|\${"0".repeat(64)}|\${event}\`);
  return {
    schema: "pytyliki.platform-evidence.v1",
    sequence: "1",
    timestamp_epoch_ns: "1000",
    provider: "linux-edac",
    event_code: 1,
    event_kind: "corrected",
    severity: "warning",
    cpu_id: null,
    source_record_id: "0",
    counter_total: "7",
    source_identity: "mc/mc0/ce_count",
    payload_sha256: payload,
    event_sha256: event,
    chain_sha256: chain,
  };
}

test("accepts a valid EDAC record and evidence chain", () => {
  const record = makeRecord();
  assert.equal(verifyEvidenceText(JSON.stringify(record)).length, 1);
});

test("rejects a modified event payload", () => {
  const record = makeRecord();
  record.counter_total = "8";
  assert.throws(() => verifyEvidenceText(JSON.stringify(record)), /digest mismatch/);
});

test("rejects a sequence gap and unsafe source identity", () => {
  const record = makeRecord();
  record.sequence = "2";
  assert.throws(() => verifyEvidenceText(JSON.stringify(record)), /sequence discontinuity/);
  const unsafe = makeRecord();
  unsafe.source_identity = "../../etc/passwd";
  assert.throws(() => verifyEvidenceText(JSON.stringify(unsafe)), /source_identity/);
});

test("rejects floating-point u64 fields", () => {
  const record = makeRecord();
  (record as unknown as { timestamp_epoch_ns: number }).timestamp_epoch_ns = 1.5;
  assert.throws(() => verifyEvidenceText(JSON.stringify(record)), /u64 values/);
});


test("verifies a worker-to-platform evidence link chain", () => {
  const source = makeRecord();
  const linkCanonical = [
    "pytyliki.ptfr-evidence-link.v1",
    "1", "1", "77", "5000", "5030", "50", "2", "65", "2",
    source.event_sha256,
  ].join("|");
  const linkDigest = hash(linkCanonical);
  const chainDigest = hash(
    \`pytyliki.ptfr-link-chain.v1|1|\${"0".repeat(64)}|\${linkDigest}\`,
  );
  const link = {
    schema: "pytyliki.ptfr-evidence-link.v1",
    sequence: "1",
    evidence_sequence: "1",
    worker_id: "77",
    worker_fault_monotonic_ns: "5000",
    mapped_evidence_monotonic_ns: "5030",
    allowed_delta_ns: "50",
    cpu_id: 2,
    fault_code: 65,
    fault_kind: 2,
    evidence_sha256: source.event_sha256,
    link_sha256: linkDigest,
    chain_sha256: chainDigest,
  };
  assert.equal(verifyLinkText(JSON.stringify(link)).length, 1);
  link.worker_id = "78";
  assert.throws(() => verifyLinkText(JSON.stringify(link)), /link digest mismatch/);
});
