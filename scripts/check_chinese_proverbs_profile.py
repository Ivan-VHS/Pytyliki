#!/usr/bin/env python3
"""Verify the 100-item Chinese-proverbs engineering crosswalk."""
from __future__ import annotations
import re
import sys
from pathlib import Path
ROOT = Path(__file__).resolve().parents[1]
DOC = ROOT / "docs" / "CHINESE_PROVERBS_100_ENGINEERING_CROSSWALK.md"
def main() -> int:
    if not DOC.is_file():
        raise SystemExit("proverbs crosswalk: FAIL: document is missing")
    text = DOC.read_text(encoding="utf-8")
    ids = [int(value) for value in re.findall(r"^\|\s*(\d{3})\s*\|", text, re.M)]
    if ids != list(range(1, 101)):
        raise SystemExit(f"proverbs crosswalk: FAIL: expected IDs 001–100 in order; got {len(ids)} rows")
    for term in ("Review required", "least privilege", "rollback", "source", "test", "CI"):
        if term.lower() not in text.lower():
            raise SystemExit(f"proverbs crosswalk: FAIL: missing required safeguard '{term}'")
    print("proverbs crosswalk: PASS (100 ordered entries and safety/evidence policy)")
    return 0
if __name__ == "__main__":
    sys.exit(main())
