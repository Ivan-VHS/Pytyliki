import { createHash } from "node:crypto";
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { fileURLToPath } from "node:url";

export type EvidenceRecord = {
  schema: string;
  sequence: string;
  timestamp_epoch_ns: string;
  provider: "windows-whea" | "linux-edac";
  event_code: number;
  event_kind: "unknown" | "corrected" | "uncorrected" | "fatal" | "counter_snapshot";
  severity: "unknown" | "info" | "warning" | "error" | "critical";
  cpu_id: number | null;
  source_record_id: string;
  counter_total: string;
  source_identity: string;
  payload_sha256: string;
  event_sha256: string;
  chain_sha256: string;
};

const SHA256 = /^[0-9a-f]{64}$/;
const DECIMAL = /^(0|[1-9][0-9]*)$/;
const SAFE_IDENTITY = /^[A-Za-z0-9._/-]+$/;
const ZERO_HASH = "0".repeat(64);

function sha256(value: string): string {
  return createHash("sha256").update(value, "utf8").digest("hex");
}

function validDecimal(value: unknown): value is string {
  if (typeof value !== "string" || !DECIMAL.test(value)) return false;
  return BigInt(value) <= 18446744073709551615n;
}

function validateShape(value: unknown, line: number): asserts value is EvidenceRecord {
  if (value === null || typeof value !== "object" || Array.isArray(value)) {
    throw new Error(\`line \${line}: record must be an object\`);
  }
  const record = value as Partial<EvidenceRecord>;
  const expected = [
    "pytyliki.platform-evidence.v1",
    "windows-whea",
    "linux-edac",
  ];
  if (record.schema !== expected[0] || !expected.includes(record.provider ?? "")) {
    throw new Error(\`line \${line}: unsupported schema or provider\`);
  }
  if (!validDecimal(record.sequence) || !validDecimal(record.timestamp_epoch_ns) ||
      !validDecimal(record.source_record_id) || !validDecimal(record.counter_total)) {
    throw new Error(\`line \${line}: u64 values must be decimal strings\`);
  }
  if (typeof record.event_code !== "number" || !Number.isInteger(record.event_code) ||
      record.event_code < 0 || record.event_code > 0xffffffff) {
    throw new Error(\`line \${line}: event_code is not a u32\`);
  }
  if (record.cpu_id !== null &&
      (typeof record.cpu_id !== "number" || !Number.isInteger(record.cpu_id) ||
       record.cpu_id < 0 || record.cpu_id > 0xfffffffe)) {
    throw new Error(\`line \${line}: cpu_id must be a u32 or null\`);
  }
  if (!["unknown", "corrected", "uncorrected", "fatal", "counter_snapshot"].includes(record.event_kind ?? "") ||
      !["unknown", "info", "warning", "error", "critical"].includes(record.severity ?? "")) {
    throw new Error(\`line \${line}: invalid event kind or severity\`);
  }
  if (typeof record.source_identity !== "string" ||
      !SAFE_IDENTITY.test(record.source_identity) ||
      record.source_identity.includes("..")) {
    throw new Error(\`line \${line}: source_identity is invalid\`);
  }
  for (const field of ["payload_sha256", "event_sha256", "chain_sha256"] as const) {
    if (typeof record[field] !== "string" || !SHA256.test(record[field])) {
      throw new Error(\`line \${line}: \${field} is not a SHA-256 digest\`);
    }
  }
}

function canonicalEvent(record: EvidenceRecord): string {
  const provider = record.provider === "windows-whea" ? "1" : "2";
  const kind = {
    unknown: "0", corrected: "1", uncorrected: "2",
    fatal: "3", counter_snapshot: "4",
  }[record.event_kind];
  const severity = {
    unknown: "0", info: "1", warning: "2", error: "3", critical: "4",
  }[record.severity];
  const cpu = record.cpu_id === null ? "4294967295" : String(record.cpu_id);
  return [
    "pytyliki.platform-evidence.v1",
    record.timestamp_epoch_ns,
    record.source_record_id,
    record.counter_total,
    provider,
    String(record.event_code),
    cpu,
    severity,
    kind,
    record.source_identity,
    record.payload_sha256,
  ].join("|");
}

export function verifyEvidenceText(text: string): EvidenceRecord[] {
  const lines = text.split(/\r?\n/).filter((line) => line.trim().length > 0);
  const records: EvidenceRecord[] = [];
  let previous = ZERO_HASH;

  for (let index = 0; index < lines.length; index += 1) {
    let parsed: unknown;
    try {
      parsed = JSON.parse(lines[index]);
    } catch {
      throw new Error(\`line \${index + 1}: malformed JSON\`);
    }
    validateShape(parsed, index + 1);
    const record = parsed as EvidenceRecord;
    const expectedSequence = String(index + 1);
    if (record.sequence !== expectedSequence) {
      throw new Error(\`line \${index + 1}: sequence discontinuity\`);
    }
    const calculatedEvent = sha256(canonicalEvent(record));
    if (calculatedEvent !== record.event_sha256) {
      throw new Error(\`line \${index + 1}: event digest mismatch\`);
    }
    const calculatedChain = sha256(
      \`pytyliki.evidence-chain.v1|\${record.sequence}|\${previous}|\${calculatedEvent}\`,
    );
    if (calculatedChain !== record.chain_sha256) {
      throw new Error(\`line \${index + 1}: chain digest mismatch\`);
    }
    previous = calculatedChain;
    records.push(record);
  }
  return records;
}

export async function verifyEvidenceFile(path: string): Promise<number> {
  const text = await readFile(path, "utf8");
  const records = verifyEvidenceText(text);
  console.log(\`PTFR evidence verified: \${records.length} record(s)\`);
  return records.length;
}

const isMain = process.argv[1] !== undefined &&
  resolve(process.argv[1]) === fileURLToPath(import.meta.url);

if (isMain) {
  const path = process.argv[2];
  if (!path) {
    console.error("Usage: node --experimental-strip-types src/verify.ts evidence.jsonl");
    process.exitCode = 2;
  } else {
    try {
      await verifyEvidenceFile(path);
    } catch (error) {
      console.error(error instanceof Error ? error.message : String(error));
      process.exitCode = 1;
    }
  }
}


export type PtfrEvidenceLink = {
  schema: "pytyliki.ptfr-evidence-link.v1";
  sequence: string;
  evidence_sequence: string;
  worker_id: string;
  worker_fault_monotonic_ns: string;
  mapped_evidence_monotonic_ns: string;
  allowed_delta_ns: string;
  cpu_id: number;
  fault_code: number;
  fault_kind: number;
  evidence_sha256: string;
  link_sha256: string;
  chain_sha256: string;
};

export function verifyLinkText(text: string): PtfrEvidenceLink[] {
  const lines = text.split(/\r?\n/).filter((line) => line.trim().length > 0);
  const links: PtfrEvidenceLink[] = [];
  let previous = ZERO_HASH;

  for (let index = 0; index < lines.length; index += 1) {
    let parsed: unknown;
    try {
      parsed = JSON.parse(lines[index]);
    } catch {
      throw new Error(\`link line \${index + 1}: malformed JSON\`);
    }
    if (parsed === null || typeof parsed !== "object" || Array.isArray(parsed)) {
      throw new Error(\`link line \${index + 1}: record must be an object\`);
    }
    const link = parsed as Partial<PtfrEvidenceLink>;
    const decimalFields = [
      link.sequence, link.evidence_sequence, link.worker_id,
      link.worker_fault_monotonic_ns, link.mapped_evidence_monotonic_ns,
      link.allowed_delta_ns,
    ];
    if (link.schema !== "pytyliki.ptfr-evidence-link.v1" ||
        !decimalFields.every(validDecimal)) {
      throw new Error(\`link line \${index + 1}: invalid schema or u64 fields\`);
    }
    if (link.sequence !== String(index + 1) ||
        BigInt(link.worker_id!) === 0n ||
        BigInt(link.allowed_delta_ns!) === 0n ||
        BigInt(link.worker_fault_monotonic_ns!) === 0n ||
        BigInt(link.mapped_evidence_monotonic_ns!) === 0n) {
      throw new Error(\`link line \${index + 1}: invalid sequence or zero identity/time\`);
    }
    if (typeof link.cpu_id !== "number" || !Number.isInteger(link.cpu_id) ||
        link.cpu_id < 0 || link.cpu_id > 0xfffffffe ||
        typeof link.fault_code !== "number" || !Number.isInteger(link.fault_code) ||
        link.fault_code <= 0 || link.fault_code > 0xffffffff ||
        typeof link.fault_kind !== "number" || ![2, 3, 5].includes(link.fault_kind)) {
      throw new Error(\`link line \${index + 1}: invalid CPU/fault identity\`);
    }
    if (typeof link.evidence_sha256 !== "string" || !SHA256.test(link.evidence_sha256) ||
        typeof link.link_sha256 !== "string" || !SHA256.test(link.link_sha256) ||
        typeof link.chain_sha256 !== "string" || !SHA256.test(link.chain_sha256)) {
      throw new Error(\`link line \${index + 1}: invalid digest\`);
    }
    const canonical = [
      "pytyliki.ptfr-evidence-link.v1",
      link.sequence, link.evidence_sequence, link.worker_id,
      link.worker_fault_monotonic_ns, link.mapped_evidence_monotonic_ns,
      link.allowed_delta_ns, String(link.cpu_id), String(link.fault_code),
      String(link.fault_kind), link.evidence_sha256,
    ].join("|");
    const calculatedLink = sha256(canonical);
    if (calculatedLink !== link.link_sha256) {
      throw new Error(\`link line \${index + 1}: link digest mismatch\`);
    }
    const calculatedChain = sha256(
      \`pytyliki.ptfr-link-chain.v1|\${link.sequence}|\${previous}|\${calculatedLink}\`,
    );
    if (calculatedChain !== link.chain_sha256) {
      throw new Error(\`link line \${index + 1}: chain digest mismatch\`);
    }
    previous = calculatedChain;
    links.push(link as PtfrEvidenceLink);
  }
  return links;
}
