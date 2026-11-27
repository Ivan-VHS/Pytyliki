#!/usr/bin/env python3
"""Validate the repository's Unified Project Contract using only the Python stdlib."""
from __future__ import annotations

import argparse
import json
import re
import sys
from pathlib import Path
from typing import Any

ROOT = Path(__file__).resolve().parents[1]
SCHEMA_PATH = ROOT / "schemas" / "unified-project-contract.v1.schema.json"
MANIFEST_PATH = ROOT / "project-contract.json"
SCHEMA_ID = "urn:ivan-vhs:unified-project-contract:v1"
CONTRACT_ID = "unified.project-contract.v1"
REPOSITORIES = {
    "pytyliki": "Ivan-VHS/Pytyliki",
    "reaper": "Ivan-VHS/Reaper",
    "oplague": "Ivan-VHS/OPlague",
    "ventusample": "Ivan-VHS/VentuSample",
}
STATUSES = {"implemented", "partial"}
INTEGRATION_STATUSES = STATUSES | {"optional", "planned"}
ID_PATTERN = re.compile(r"^[a-z0-9]+(?:[.-][a-z0-9]+)*$")


def _unique_object(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise ValueError(f"duplicate JSON property: {key}")
        result[key] = value
    return result


def _load(path: Path) -> Any:
    try:
        return json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_unique_object,
            parse_constant=lambda value: (_ for _ in ()).throw(
                ValueError(f"non-finite JSON value: {value}")
            ),
        )
    except OSError as error:
        raise ValueError(f"cannot read {path.relative_to(ROOT)}: {error}") from error
    except json.JSONDecodeError as error:
        raise ValueError(f"invalid JSON in {path.relative_to(ROOT)}: {error}") from error


def _require(condition: bool, message: str) -> None:
    if not condition:
        raise ValueError(message)


def _string(value: Any, label: str) -> None:
    _require(isinstance(value, str) and bool(value.strip()), f"{label} must be a non-empty string")


def _strings(value: Any, label: str, *, minimum: int = 0) -> None:
    _require(isinstance(value, list), f"{label} must be an array")
    _require(len(value) >= minimum, f"{label} must contain at least {minimum} item(s)")
    _require(all(isinstance(item, str) and item.strip() for item in value),
             f"{label} must contain only non-empty strings")
    _require(len(value) == len(set(value)), f"{label} must not contain duplicates")


def _evidence(value: Any, label: str) -> None:
    _strings(value, label, minimum=1)
    for item in value:
        relative = Path(item)
        _require(not relative.is_absolute() and ".." not in relative.parts,
                 f"{label} contains an unsafe repository path: {item}")
        target = (ROOT / relative).resolve()
        _require(target.is_relative_to(ROOT.resolve()),
                 f"{label} escapes the repository: {item}")
        _require(target.is_file(), f"{label} references a missing file: {item}")


def _validate_supabase_adapter(project_id: str) -> dict[str, Any]:
    """Validate the opt-in, offline-by-default Supabase adapter contract."""
    schema_path = ROOT / "federation" / "schemas" / "supabase-adapter.v1.schema.json"
    adapter_path = ROOT / "federation" / "supabase-adapter.v1.json"
    schema = _load(schema_path)
    adapter = _load(adapter_path)

    _require(isinstance(schema, dict), "Supabase schema root must be an object")
    _require(schema.get("$schema") == "https://json-schema.org/draft/2020-12/schema",
             "Supabase schema must use JSON Schema 2020-12")
    _require(schema.get("$id") == "https://schemas.ivan-vhs.dev/supabase-adapter.v1.schema.json",
             "unexpected Supabase schema identifier")
    _require(schema.get("title") == "Ivan-VHS Supabase Adapter Contract v1",
             "unexpected Supabase schema title")
    _require(schema.get("type") == "object" and schema.get("additionalProperties") is False,
             "Supabase schema must reject unknown root properties")

    fields = {
        "schema", "project", "repository", "role", "status", "default_mode",
        "supabase", "network_policy", "local_first", "contracts",
        "data_domains", "excluded_data", "security",
    }
    _require(set(schema.get("required", [])) == fields,
             "Supabase schema required fields do not match adapter v1")
    _require(isinstance(adapter, dict), "Supabase adapter root must be an object")
    _require(set(adapter) == fields, "Supabase adapter contains missing or unknown fields")
    _require(adapter["schema"] == "ivan-vhs.supabase-adapter.v1",
             "unsupported Supabase adapter schema")
    _require(project_id in REPOSITORIES, f"unknown project id for Supabase adapter: {project_id}")
    _require(adapter["project"] == project_id, "Supabase adapter project identity does not match")
    _require(adapter["repository"] == REPOSITORIES[project_id],
             "Supabase adapter repository does not match project identity")
    _require(adapter["role"] in {"control-plane", "producer", "consumer", "producer-consumer"},
             "Supabase adapter role is invalid")
    _require(adapter["status"] == "contract-only",
             "Supabase adapter must remain contract-only until hosted integration is verified")
    _require(adapter["default_mode"] == "offline-only",
             "Supabase adapter must remain offline-only by default")

    sections = {
        "supabase": {
            "deployment", "url_env", "publishable_key_env", "service_role_key_in_client",
        },
        "network_policy": {
            "enabled_by_default", "background_sync", "requires_user_action",
        },
        "local_first": {
            "local_data_authoritative", "sync_model", "conflict_resolution",
        },
        "contracts": {"event_envelope", "artifact_manifest"},
        "security": {
            "rls_required", "service_role_key_never_in_client",
            "explicit_upload_consent", "telemetry_default",
        },
    }
    for name, expected_keys in sections.items():
        _require(isinstance(adapter[name], dict), f"Supabase {name} must be an object")
        _require(set(adapter[name]) == expected_keys,
                 f"Supabase {name} has missing or unknown fields")

    supabase = adapter["supabase"]
    _require(supabase["deployment"] == "not-configured",
             "Supabase hosted deployment is not configured")
    _require(supabase["url_env"] == "SUPABASE_URL",
             "Supabase URL must be supplied through SUPABASE_URL")
    _require(supabase["publishable_key_env"] == "SUPABASE_PUBLISHABLE_KEY",
             "Supabase publishable key must use SUPABASE_PUBLISHABLE_KEY")
    _require(supabase["service_role_key_in_client"] is False,
             "service-role keys must never be placed in a client")

    network = adapter["network_policy"]
    _require(network["enabled_by_default"] is False,
             "Supabase network access must remain disabled by default")
    _require(network["background_sync"] is False,
             "Supabase background synchronization must remain disabled")
    _require(network["requires_user_action"] is True,
             "Supabase synchronization must require explicit user action")

    local = adapter["local_first"]
    _require(local["local_data_authoritative"] is True,
             "local data must remain authoritative")
    _require(local["sync_model"] == "append-only-event-log",
             "Supabase synchronization must use the versioned append-only event log")
    _require(local["conflict_resolution"] == "domain-specific-versioned",
             "Supabase conflicts must use domain-specific versioned resolution")

    contracts = adapter["contracts"]
    _require(contracts["event_envelope"] == "ivan-vhs.ecosystem-event.v1",
             "unexpected ecosystem event envelope")
    _require(contracts["artifact_manifest"] == "ivan-vhs.ecosystem-artifact.v1",
             "unexpected ecosystem artifact manifest")

    _strings(adapter["data_domains"], "supabase.data_domains", minimum=1)
    _strings(adapter["excluded_data"], "supabase.excluded_data", minimum=1)

    security = adapter["security"]
    _require(security["rls_required"] is True, "Supabase Row Level Security is mandatory")
    _require(security["service_role_key_never_in_client"] is True,
             "service-role keys must never be exposed to clients")
    _require(security["explicit_upload_consent"] is True,
             "remote upload requires explicit user consent")
    _require(security["telemetry_default"] == "disabled",
             "remote telemetry must remain disabled by default")

    return {
        "valid": True,
        "status": adapter["status"],
        "default_mode": adapter["default_mode"],
        "network_enabled_by_default": network["enabled_by_default"],
    }


def validate() -> dict[str, Any]:
    schema = _load(SCHEMA_PATH)
    _require(isinstance(schema, dict), "schema root must be an object")
    _require(schema.get("$schema") == "https://json-schema.org/draft/2020-12/schema",
             "schema must declare JSON Schema 2020-12")
    _require(schema.get("$id") == SCHEMA_ID, "unexpected schema identifier")
    _require(schema.get("title") == "Ivan-VHS Unified Project Contract v1",
             "unexpected schema title")

    manifest = _load(MANIFEST_PATH)
    _require(isinstance(manifest, dict), "manifest root must be an object")
    expected_root = {
        "schema", "project", "implementation", "capabilities", "interfaces",
        "integrations", "safety_boundaries", "extension_points",
    }
    _require(set(manifest) == expected_root, "manifest root fields do not match contract v1")
    _require(manifest["schema"] == CONTRACT_ID, f"schema must be {CONTRACT_ID}")

    project = manifest["project"]
    _require(isinstance(project, dict), "project must be an object")
    _require(set(project) == {"id", "name", "repository", "summary", "documentation"},
             "project fields do not match contract v1")
    project_id = project["id"]
    _require(project_id in REPOSITORIES, f"unknown project id: {project_id}")
    _string(project["name"], "project.name")
    _require(project["repository"] == REPOSITORIES[project_id],
             "project.repository does not match project.id")
    _string(project["summary"], "project.summary")
    _evidence([project["documentation"]], "project.documentation")

    implementation = manifest["implementation"]
    _require(isinstance(implementation, dict), "implementation must be an object")
    _require(set(implementation) == {"languages", "runtime_platforms", "desktop_platforms"},
             "implementation fields do not match contract v1")
    _strings(implementation["languages"], "implementation.languages", minimum=1)
    _strings(implementation["runtime_platforms"], "implementation.runtime_platforms", minimum=1)
    _strings(implementation["desktop_platforms"], "implementation.desktop_platforms")

    capabilities = manifest["capabilities"]
    _require(isinstance(capabilities, list) and bool(capabilities),
             "capabilities must be a non-empty array")
    capability_ids: set[str] = set()
    for index, item in enumerate(capabilities):
        label = f"capabilities[{index}]"
        _require(isinstance(item, dict), f"{label} must be an object")
        _require(set(item) == {"id", "status", "summary", "evidence"},
                 f"{label} fields do not match contract v1")
        _string(item["id"], f"{label}.id")
        _require(ID_PATTERN.fullmatch(item["id"]) is not None, f"{label}.id is not canonical")
        _require(item["id"] not in capability_ids, f"duplicate capability id: {item['id']}")
        capability_ids.add(item["id"])
        _require(item["status"] in STATUSES, f"{label}.status must be implemented or partial")
        _string(item["summary"], f"{label}.summary")
        _evidence(item["evidence"], f"{label}.evidence")

    interfaces = manifest["interfaces"]
    _require(isinstance(interfaces, list), "interfaces must be an array")
    interface_ids: set[str] = set()
    for index, item in enumerate(interfaces):
        label = f"interfaces[{index}]"
        _require(isinstance(item, dict), f"{label} must be an object")
        _require(set(item) == {"id", "format", "direction", "status", "evidence"},
                 f"{label} fields do not match contract v1")
        _string(item["id"], f"{label}.id")
        _require(ID_PATTERN.fullmatch(item["id"]) is not None, f"{label}.id is not canonical")
        _require(item["id"] not in interface_ids, f"duplicate interface id: {item['id']}")
        interface_ids.add(item["id"])
        _string(item["format"], f"{label}.format")
        _require(item["direction"] in {"provides", "consumes", "both"},
                 f"{label}.direction is invalid")
        _require(item["status"] in STATUSES, f"{label}.status is invalid")
        _evidence(item["evidence"], f"{label}.evidence")

    integrations = manifest["integrations"]
    _require(isinstance(integrations, list), "integrations must be an array")
    integration_keys: set[tuple[str, str, str]] = set()
    for index, item in enumerate(integrations):
        label = f"integrations[{index}]"
        _require(isinstance(item, dict), f"{label} must be an object")
        _require(set(item) == {"project", "interface", "mode", "status", "opt_in", "summary"},
                 f"{label} fields do not match contract v1")
        _require(item["project"] in REPOSITORIES and item["project"] != project_id,
                 f"{label}.project must identify another ecosystem project")
        _string(item["interface"], f"{label}.interface")
        _require(ID_PATTERN.fullmatch(item["interface"]) is not None,
                 f"{label}.interface is not canonical")
        _require(item["mode"] in {"native-ffi", "json-file", "shared-contract"},
                 f"{label}.mode is invalid")
        _require(item["status"] in INTEGRATION_STATUSES, f"{label}.status is invalid")
        _require(isinstance(item["opt_in"], bool), f"{label}.opt_in must be boolean")
        _string(item["summary"], f"{label}.summary")
        key = (item["project"], item["interface"], item["mode"])
        _require(key not in integration_keys, f"duplicate integration: {key}")
        integration_keys.add(key)

    _strings(manifest["safety_boundaries"], "safety_boundaries", minimum=1)
    _strings(manifest["extension_points"], "extension_points", minimum=1)

    _validate_supabase_adapter(project_id)

    return {
        "valid": True,
        "contract": CONTRACT_ID,
        "project": project["name"],
        "repository": project["repository"],
        "capabilities": len(capabilities),
        "interfaces": len(interfaces),
        "integrations": len(integrations),
    }


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--json", action="store_true", help="print a machine-readable validation result")
    args = parser.parse_args()
    try:
        result = validate()
    except (ValueError, TypeError, KeyError) as error:
        print(f"project-contract: INVALID: {error}", file=sys.stderr)
        return 1
    if args.json:
        print(json.dumps(result, ensure_ascii=False, sort_keys=True, indent=2))
    else:
        print(
            f"Project contract valid: {result['project']} | "
            f"{result['capabilities']} capabilities | "
            f"{result['interfaces']} interfaces | "
            f"{result['integrations']} integrations"
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
