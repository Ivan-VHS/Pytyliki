#!/usr/bin/env python3
"""Validate balanced Markdown fences and local links."""
from __future__ import annotations
import argparse
import re
from pathlib import Path
from urllib.parse import unquote, urlsplit

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--target", required=True)
    args = parser.parse_args()
    target = Path(args.target).resolve()
    content = target.read_text(encoding="utf-8")
    fence_count = content.count(chr(96) * 3)
    if fence_count % 2:
        raise SystemExit("Unclosed Markdown code fence: {}".format(target))
    problems = []
    for raw in re.findall(r"!?\\[[^\\]]*\\]\\(([^)]+)\\)", content):
        url = raw.strip().split()[0].strip("<>")
        parsed = urlsplit(url)
        if parsed.scheme or url.startswith("#") or url.startswith("mailto:"):
            continue
        path = unquote(parsed.path)
        if path and not (target.parent / path).resolve().exists():
            problems.append("broken local link: {}".format(url))
    if problems:
        raise SystemExit("\n".join(problems))
    print("Markdown invariants passed: {}".format(target))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
