#!/usr/bin/env python3
"""Keep the compiled Pytyliki diagnostic registry and its Markdown help in sync."""
from __future__ import annotations
import re
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "errors.c"
DOCS = ROOT / "docs" / "errors"
CODE = re.compile(r'"(PYT-[EW][0-9]{4})"')
def main() -> int:
    source_codes = set(CODE.findall(SOURCE.read_text(encoding="utf-8")))
    documented_codes = set()
    for path in DOCS.glob("PYT-*.md"):
        match = re.search(r"^#\s+(PYT-[EW][0-9]{4})\b", path.read_text(encoding="utf-8"), re.MULTILINE)
        if match:
            documented_codes.add(match.group(1))
    missing = sorted(source_codes - documented_codes)
    orphaned = sorted(documented_codes - source_codes)
    if missing or orphaned:
        if missing: print("Missing error documentation:", ", ".join(missing))
        if orphaned: print("Documentation without a registry entry:", ", ".join(orphaned))
        return 1
    if not source_codes:
        print("The Pytyliki error registry is empty.")
        return 1
    print(f"Pytyliki error registry: PASS ({len(source_codes)} documented codes)")
    return 0
if __name__ == "__main__":
    raise SystemExit(main())
