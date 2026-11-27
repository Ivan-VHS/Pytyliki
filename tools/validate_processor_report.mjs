#!/usr/bin/env node
import { readFileSync } from "node:fs";

function fail(message) {
  console.error("Pytyliki report validation failed: " + message);
  process.exit(1);
}
const [path] = process.argv.slice(2);
if (!path) fail("usage: node tools/validate_processor_report.mjs <report.json>");

let report;
try { report = JSON.parse(readFileSync(path, "utf8")); }
catch (error) { fail("cannot parse report: " + error.message); }

const requireValue = (condition, message) => { if (!condition) fail(message); };
const object = value => value !== null && typeof value === "object" && !Array.isArray(value);
const text = value => typeof value === "string" && value.length > 0;
const number = value => typeof value === "number" && Number.isFinite(value);
const integer = value => number(value) && Number.isInteger(value);

requireValue(object(report), "top-level report must be an object");
requireValue(report.schema === "pytyliki.report.v1", "unsupported report schema");
requireValue(object(report.program) && report.program.name === "Pytyliki" && text(report.program.version), "invalid producer identity");
const profile = report.profile;
requireValue(object(profile), "profile is required");
for (const field of ["architecture", "cpu", "vendor"]) requireValue(text(profile[field]), "profile." + field + " is required");
for (const field of ["logical_processors", "family", "model_id", "physical_memory_mib", "available_memory_mib"]) {
  requireValue(integer(profile[field]) && profile[field] >= 0, "profile." + field + " must be a non-negative integer");
}
requireValue(profile.logical_processors > 0 && profile.physical_memory_mib > 0, "profile processor and memory capacities must be positive");
requireValue(profile.available_memory_mib <= profile.physical_memory_mib, "available memory cannot exceed physical memory");
const isa = profile.isa;
requireValue(object(isa), "profile.isa is required");
for (const field of ["sse2", "sse4_2", "avx_hardware", "osxsave", "avx_usable", "avx2_usable", "avx512f_usable", "aes", "fma_usable", "virtualization"]) {
  requireValue(typeof isa[field] === "boolean", "profile.isa." + field + " must be boolean");
}
requireValue(!isa.avx_usable || (isa.avx_hardware && isa.osxsave), "usable AVX requires hardware support and OS-managed vector state");
requireValue(!isa.avx2_usable || isa.avx_usable, "usable AVX2 requires usable AVX");
requireValue(!isa.avx512f_usable || isa.avx_usable, "usable AVX-512 requires usable AVX");

const suite = report.suite;
requireValue(object(suite) && suite.schema === "pytyliki.suite.v1", "suite schema is missing or unsupported");
for (const field of ["repeat", "warmup_runs", "measured_runs"]) requireValue(integer(suite[field]), "suite." + field + " must be an integer");
requireValue(suite.repeat >= 1 && suite.repeat <= 100, "repeat must be in 1..100");
requireValue(suite.warmup_runs >= 0 && suite.warmup_runs < suite.repeat, "warmup count must leave measured runs");
requireValue(suite.measured_runs === suite.repeat - suite.warmup_runs && suite.measured_runs > 0, "measured run count is inconsistent");
requireValue(Array.isArray(suite.runs) && suite.runs.length === suite.repeat, "run count must match repeat");
suite.runs.forEach((run, index) => {
  requireValue(object(run) && run.run === index + 1, "run sequence is not contiguous");
  requireValue(typeof run.warmup === "boolean" && run.warmup === (index < suite.warmup_runs), "warmup marker is inconsistent");
  for (const field of ["iterations", "memory_mib", "passes"]) requireValue(integer(run[field]) && run[field] > 0, "run." + field + " must be a positive integer");
  for (const field of ["integer_ms", "memory_ms", "matrix_ms"]) requireValue(number(run[field]) && run[field] >= 0, "run." + field + " must be finite and non-negative");
  requireValue(integer(run.checksum) && run.checksum >= 0, "run.checksum must be a non-negative integer");
  requireValue(typeof run.integrity_ok === "boolean", "run.integrity_ok must be boolean");
});
requireValue(object(suite.statistics), "suite.statistics is required");
for (const domain of ["integer_ms", "memory_ms", "matrix_ms"]) {
  const values = suite.statistics[domain];
  requireValue(object(values), "suite.statistics." + domain + " is required");
  for (const value of Object.values(values)) requireValue(number(value) && value >= 0, "suite.statistics." + domain + " has a non-finite or negative value");
}
requireValue(text(suite.timing_note), "suite.timing_note is required");

console.log("PYTYLIKI_PROCESSOR_REPORT_SCHEMA_OK");
console.log("PYTYLIKI_PROCESSOR_REPORT_RUNS_OK:" + suite.runs.length);
console.log("PYTYLIKI_PROCESSOR_REPORT_ISA_OK");
