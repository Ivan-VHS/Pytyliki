export type RasSource = "windows-whea" | "linux-edac";
export type RasSeverity = "unknown" | "corrected" | "uncorrected";
export type ClockDomain = "unix-realtime" | "process-monotonic";

export interface RasEvent {
  recordId: number;
  timestampNs: number;
  clockDomain: ClockDomain;
  source: RasSource;
  eventId: number;
  level: number;
  severity: RasSeverity;
  countDelta: number;
  controllerId: number;
  flags: number;
  attribution: "system-wide";
}

export interface RasProviderReport {
  schema: "pytyliki.ras-provider-report.v1";
  provider: string;
  status: "ok" | "unsupported";
  read_only: true;
  hardware_writes?: false;
  worker_attribution?: "none" | "not_inferred";
  events: RasEvent[];
  droppedRecords?: number;
  mayHaveMore?: boolean;
}

export interface RasSummary {
  events: number;
  correctedDelta: number;
  uncorrectedDelta: number;
  unknownSeverity: number;
  baselineRecords: number;
  droppedRecords: number;
}

export type ValidationResult =
  | { ok: true; report: RasProviderReport; summary: RasSummary }
  | { ok: false; error: string };

const MAX_SAFE = Number.MAX_SAFE_INTEGER;
const BASELINE_FLAG = 1 << 2;

function addSafe(left: number, right: number): number | null {
  const sum = left + right;
  return Number.isSafeInteger(sum) ? sum : null;
}

function integerInRange(value: unknown, maximum = MAX_SAFE): value is number {
  return typeof value === "number" &&
    Number.isSafeInteger(value) && value >= 0 && value <= maximum;
}

export function validateRasProviderReport(input: unknown): ValidationResult {
  if (typeof input !== "object" || input === null || Array.isArray(input)) {
    return { ok: false, error: "report must be an object" };
  }
  const report = input as Record<string, unknown>;
  if (report.schema !== "pytyliki.ras-provider-report.v1" ||
      typeof report.provider !== "string" ||
      report.provider.length === 0 || report.provider.length > 160 ||
      (report.status !== "ok" && report.status !== "unsupported") ||
      report.read_only !== true ||
      report.hardware_writes === true ||
      !Array.isArray(report.events) ||
      report.events.length > 32) {
    return { ok: false, error: "report header or schema is invalid" };
  }
  if (report.status === "unsupported" && report.events.length !== 0) {
    return { ok: false, error: "unsupported provider cannot report events" };
  }
  const events: RasEvent[] = [];
  let correctedDelta = 0;
  let uncorrectedDelta = 0;
  let unknownSeverity = 0;
  let baselineRecords = 0;
  const seen = new Set<string>();

  for (const candidate of report.events) {
    if (typeof candidate !== "object" || candidate === null || Array.isArray(candidate)) {
      return { ok: false, error: "event must be an object" };
    }
    const event = candidate as Record<string, unknown>;
    const valid =
      integerInRange(event.recordId, MAX_SAFE) && event.recordId > 0 &&
      integerInRange(event.timestampNs) &&
      (event.clockDomain === "unix-realtime" || event.clockDomain === "process-monotonic") &&
      (event.source === "windows-whea" || event.source === "linux-edac") &&
      integerInRange(event.eventId, 0xffffffff) && event.eventId > 0 &&
      integerInRange(event.level, 0xffffffff) &&
      (event.severity === "unknown" || event.severity === "corrected" || event.severity === "uncorrected") &&
      integerInRange(event.countDelta) && event.countDelta > 0 &&
      integerInRange(event.controllerId, 0xffffffff) &&
      integerInRange(event.flags, 0xffffffff) &&
      event.attribution === "system-wide";
    if (!valid) return { ok: false, error: "event fields or attribution are invalid" };
    if (event.source === "linux-edac" &&
        (event.clockDomain !== "process-monotonic" ||
         (event.severity !== "corrected" && event.severity !== "uncorrected"))) {
      return { ok: false, error: "Linux EDAC event has an invalid clock or severity" };
    }
    if (event.source === "windows-whea" && event.clockDomain !== "unix-realtime") {
      return { ok: false, error: "WHEA event has an invalid clock domain" };
    }
    const recordId = event.recordId as number;
    const source = event.source as RasSource;
    const controllerId = event.controllerId as number;
    const recordKey = `${source}:${controllerId}:${recordId}`;
    if (seen.has(recordKey)) return { ok: false, error: "duplicate provider record" };
    seen.add(recordKey);

    const normalized: RasEvent = {
      recordId: event.recordId as number,
      timestampNs: event.timestampNs as number,
      clockDomain: event.clockDomain as ClockDomain,
      source: event.source as RasSource,
      eventId: event.eventId as number,
      level: event.level as number,
      severity: event.severity as RasSeverity,
      countDelta: event.countDelta as number,
      controllerId: event.controllerId as number,
      flags: event.flags as number,
      attribution: "system-wide"
    };
    events.push(normalized);
    if ((normalized.flags & BASELINE_FLAG) !== 0) {
      baselineRecords += 1;
    } else if (normalized.severity === "corrected") {
      const sum = addSafe(correctedDelta, normalized.countDelta);
      if (sum === null) return { ok: false, error: "corrected counter total exceeds JavaScript safe integer range" };
      correctedDelta = sum;
    } else if (normalized.severity === "uncorrected") {
      const sum = addSafe(uncorrectedDelta, normalized.countDelta);
      if (sum === null) return { ok: false, error: "uncorrected counter total exceeds JavaScript safe integer range" };
      uncorrectedDelta = sum;
    } else {
      unknownSeverity += 1;
    }
  }

  if (report.droppedRecords !== undefined &&
      !integerInRange(report.droppedRecords, 0xffffffff)) {
    return { ok: false, error: "droppedRecords is invalid" };
  }
  const summary: RasSummary = {
    events: events.length,
    correctedDelta,
    uncorrectedDelta,
    unknownSeverity,
    baselineRecords,
    droppedRecords: (report.droppedRecords as number | undefined) ?? 0
  };
  return {
    ok: true,
    report: { ...report, events } as unknown as RasProviderReport,
    summary
  };
}
