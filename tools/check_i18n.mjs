#!/usr/bin/env node
import { readFile } from "node:fs/promises";
import { resolve } from "node:path";
import { fileURLToPath } from "node:url";

const root = resolve(fileURLToPath(new URL("..", import.meta.url)));
const manifestSchema = "pytyliki.i18n-manifest/v1";
const catalogSchema = "pytyliki.i18n-catalog-set/v1";
const schemaId = "https://github.com/Ivan-VHS/Pytyliki/blob/main/i18n/catalog-set.v1.schema.json";
const statuses = new Set(["source", "complete", "partial", "machine-draft", "needs-review"]);
const fail = (message) => {
  console.error("i18n: " + message);
  process.exitCode = 1;
};
const readJson = async (path) => JSON.parse(await readFile(resolve(root, path), "utf8"));
const isRecord = (value) => value !== null && typeof value === "object" && !Array.isArray(value);

const canonical = (tag) => {
  if (typeof tag !== "string" || !tag.trim()) throw new Error("locale tag must be a non-empty string");
  const values = Intl.getCanonicalLocales(tag);
  if (values.length !== 1) throw new Error("locale tag must resolve to exactly one canonical tag");
  return values[0];
};

const placeholders = (message) => [...message.matchAll(/\{([A-Za-z0-9_.-]+)\}/g)]
  .map((match) => match[1])
  .sort();

try {
  const manifest = await readJson("i18n/manifest.v1.json");
  const set = await readJson("i18n/catalogs.v1.json");
  const schema = await readJson("i18n/catalog-set.v1.schema.json");

  if (!isRecord(manifest) || !isRecord(set) || !isRecord(schema)) {
    fail("manifest, catalog set and schema must be JSON objects");
  }
  if (manifest.schema !== manifestSchema) fail("unsupported manifest schema");
  if (set.schema !== catalogSchema) fail("unsupported catalog-set schema");
  if (schema.$id !== schemaId) fail("catalog schema $id does not identify the Pytyliki schema");
  if (schema.properties?.schema?.const !== set.schema) fail("JSON schema and catalog-set schema constants differ");
  if (manifest.project !== "Pytyliki") fail("manifest project must be Pytyliki");
  if (manifest.catalogSet !== "catalogs.v1.json") fail("manifest catalogSet must identify catalogs.v1.json");
  if (manifest.sourceLocale !== set.sourceLocale) fail("source locale differs between manifest and catalog set");
  if (!manifest.languageTags || manifest.languageTags.acceptAnyWellFormedTag !== true) {
    fail("manifest must accept any well-formed BCP 47 tag");
  }
  if (!isRecord(manifest.catalogs) || !isRecord(set.locales)) {
    fail("manifest catalogs and catalog-set locales must be objects");
  }
  if (!Array.isArray(manifest.fallback) ||
      manifest.fallback.length < 4 ||
      manifest.fallback[0] !== "exact-tag" ||
      manifest.fallback[1] !== "script-parent" ||
      manifest.fallback[2] !== "language-parent" ||
      manifest.fallback.at(-1) !== manifest.sourceLocale) {
    fail("fallback order must be exact tag, script parent, language parent, then source locale");
  }

  let canonicalSource = null;
  try {
    canonicalSource = canonical(manifest.sourceLocale);
    if (canonicalSource !== manifest.sourceLocale) {
      fail("source locale is not canonical: use " + canonicalSource);
    }
  } catch (error) {
    fail("invalid source locale: " + error.message);
  }

  const source = set.locales?.[manifest.sourceLocale];
  if (!isRecord(source) || !isRecord(source.messages)) {
    fail("source catalog has no messages object");
  }
  const sourceMessages = isRecord(source?.messages) ? source.messages : {};
  const sourceKeys = Object.keys(sourceMessages).sort();
  if (sourceKeys.length === 0) fail("source catalog is empty");

  const manifestTags = Object.keys(manifest.catalogs ?? {});
  const catalogTags = Object.keys(set.locales ?? {});
  const canonicalTags = new Set();

  for (const [tag, entry] of Object.entries(manifest.catalogs ?? {})) {
    let canonicalTag;
    try {
      canonicalTag = canonical(tag);
    } catch (error) {
      fail("invalid BCP 47 tag in manifest: " + tag + " (" + error.message + ")");
      continue;
    }
    if (canonicalTag !== tag) fail("locale tag is not canonical: " + tag + "; use " + canonicalTag);
    if (canonicalTags.has(canonicalTag)) fail("duplicate canonical locale tag: " + canonicalTag);
    canonicalTags.add(canonicalTag);

    if (!isRecord(entry)) {
      fail("manifest entry for " + tag + " is invalid");
      continue;
    }
    if (typeof entry.file !== "string" || entry.file !== tag) {
      fail("manifest catalog key/file mismatch for " + tag);
    }
    if (!statuses.has(entry.status)) fail("unsupported manifest translation status for " + tag);

    const bundle = set.locales?.[tag];
    if (!isRecord(bundle)) {
      fail("catalog is missing for " + tag);
      continue;
    }
    if (typeof bundle.name !== "string" || !bundle.name.trim()) {
      fail("catalog name is empty or invalid for " + tag);
    }
    if (!["ltr", "rtl"].includes(bundle.direction)) {
      fail("catalog direction must be ltr or rtl for " + tag);
    }
    if (!statuses.has(bundle.status)) fail("unsupported catalog translation status for " + tag);
    if (bundle.status !== entry.status) fail("manifest/catalog status mismatch for " + tag);
    if (!isRecord(bundle.messages)) {
      fail("messages object is missing for " + tag);
      continue;
    }

    const keys = Object.keys(bundle.messages).sort();
    for (const key of keys) {
      const value = bundle.messages[key];
      if (!key.trim() || typeof value !== "string" || !value.trim()) {
        fail("empty key or non-string/empty message: " + tag + ":" + key);
        continue;
      }
      if (value.includes("\uFFFD")) fail("replacement character in " + tag + ":" + key);
      if (!Object.hasOwn(sourceMessages, key)) fail("unknown message id: " + tag + ":" + key);
      if (Object.hasOwn(sourceMessages, key) &&
          placeholders(sourceMessages[key]).join("|") !== placeholders(value).join("|")) {
        fail("placeholder set differs from source: " + tag + ":" + key);
      }
    }

    const extra = keys.filter((key) => !Object.hasOwn(sourceMessages, key));
    if (extra.length) fail(tag + " contains unknown message ids: " + extra.join(", "));
    if (entry.status === "complete") {
      const missing = sourceKeys.filter((key) => !Object.hasOwn(bundle.messages, key));
      if (missing.length) fail(tag + " is marked complete but messages are missing: " + missing.join(", "));
    }
    if (tag === manifest.sourceLocale && (entry.status !== "source" || bundle.status !== "source")) {
      fail("source locale must have status=source in both manifest and catalog set");
    }
  }

  for (const tag of catalogTags) {
    let canonicalTag;
    try {
      canonicalTag = canonical(tag);
    } catch (error) {
      fail("invalid BCP 47 tag in catalog set: " + tag + " (" + error.message + ")");
      continue;
    }
    if (canonicalTag !== tag) fail("catalog locale tag is not canonical: " + tag + "; use " + canonicalTag);
    if (!Object.hasOwn(manifest.catalogs ?? {}, tag)) fail("catalog " + tag + " is not registered in manifest");
  }
  for (const tag of manifestTags) {
    if (!Object.hasOwn(set.locales ?? {}, tag)) fail("manifest locale has no catalog: " + tag);
  }

  if (typeof Intl.Segmenter !== "function") {
    fail("runtime lacks Intl.Segmenter; grapheme-aware text editing cannot be verified");
  }
  for (const tag of manifestTags) {
    try {
      new Intl.NumberFormat(tag).format(12345.67);
      new Intl.DateTimeFormat(tag).format(new Date("2026-01-02T03:04:05Z"));
      new Intl.PluralRules(tag).select(2);
    } catch (error) {
      fail("Intl locale services failed for " + tag + ": " + error.message);
    }
  }

  if (process.exitCode) process.exit(process.exitCode);
  console.log(
    "i18n: " + manifest.project + " passed; " + manifestTags.length +
    " catalogs, " + sourceKeys.length + " stable message ids; open BCP 47 tag policy enabled."
  );
} catch (error) {
  fail(error.stack ?? String(error));
}
