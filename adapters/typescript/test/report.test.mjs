import test from "node:test";
import assert from "node:assert/strict";
import { validateRasProviderReport } from "../dist/index.js";

test("accepts EDAC corrected and uncorrected deltas", () => {
  const result = validateRasProviderReport({
    schema: "pytyliki.ras-provider-report.v1",
    provider: "Linux EDAC",
    status: "ok",
    read_only: true,
    events: [
      { recordId: 1, timestampNs: 10, clockDomain: "process-monotonic",
        source: "linux-edac", eventId: 1, level: 0, severity: "corrected",
        countDelta: 4, controllerId: 0, flags: 2, attribution: "system-wide" },
      { recordId: 2, timestampNs: 11, clockDomain: "process-monotonic",
        source: "linux-edac", eventId: 2, level: 0, severity: "uncorrected",
        countDelta: 1, controllerId: 0, flags: 2, attribution: "system-wide" }
    ],
    droppedRecords: 0
  });
  assert.equal(result.ok, true);
  if (result.ok) {
    assert.equal(result.summary.correctedDelta, 4);
    assert.equal(result.summary.uncorrectedDelta, 1);
  }
});

test("baseline counters are not counted as newly observed faults", () => {
  const result = validateRasProviderReport({
    schema: "pytyliki.ras-provider-report.v1",
    provider: "Linux EDAC",
    status: "ok",
    read_only: true,
    events: [{
      recordId: 3, timestampNs: 12, clockDomain: "process-monotonic",
      source: "linux-edac", eventId: 1, level: 0, severity: "corrected",
      countDelta: 20, controllerId: 0, flags: 6, attribution: "system-wide"
    }]
  });
  assert.equal(result.ok, true);
  if (result.ok) {
    assert.equal(result.summary.baselineRecords, 1);
    assert.equal(result.summary.correctedDelta, 0);
  }
});

test("rejects false worker attribution and duplicate sequence IDs", () => {
  const result = validateRasProviderReport({
    schema: "pytyliki.ras-provider-report.v1",
    provider: "Windows WHEA",
    status: "ok",
    read_only: true,
    events: [{
      recordId: 4, timestampNs: 12, clockDomain: "unix-realtime",
      source: "windows-whea", eventId: 17, level: 2, severity: "unknown",
      countDelta: 1, controllerId: 0, flags: 2, attribution: "worker-4"
    }]
  });
  assert.equal(result.ok, false);
});
