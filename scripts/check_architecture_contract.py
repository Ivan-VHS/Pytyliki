#!/usr/bin/env python3
"""Static guard for Pytyliki's documented modular-C11 architecture contract."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"architecture contract: FAIL: {message}")


def main() -> int:
    cmake_path = ROOT / "CMakeLists.txt"
    architecture_path = ROOT / "docs" / "ARCHITECTURE.md"
    contract_path = ROOT / "docs" / "ARCHITECTURE_66.md"
    evolution_path = ROOT / "docs" / "RECURSIVE_EVOLUTION.md"

    for path in (cmake_path, architecture_path, contract_path, evolution_path):
        require(path.is_file(), f"required file is missing: {path.relative_to(ROOT)}")

    cmake = cmake_path.read_text(encoding="utf-8")
    architecture = architecture_path.read_text(encoding="utf-8").lower()
    contract = contract_path.read_text(encoding="utf-8").lower()
    evolution = evolution_path.read_text(encoding="utf-8").lower()

    require(bool(re.search(r"set\s*\(\s*CMAKE_C_STANDARD\s+11\s*\)", cmake, re.I)),
            "CMake must explicitly select C11")
    require(bool(re.search(r"set\s*\(\s*CMAKE_C_STANDARD_REQUIRED\s+ON\s*\)", cmake, re.I)),
            "CMake must require the selected C standard")
    require(bool(re.search(r"set\s*\(\s*CMAKE_C_EXTENSIONS\s+OFF\s*\)", cmake, re.I)),
            "CMake extensions must remain disabled")
    require("add_library(pytyliki_core" in cmake, "shared pytyliki_core library is missing")
    require("src/policy.c" in cmake, "policy module must be linked into the core")
    require("include(ctest)" in cmake.lower() and "if(build_testing)" in cmake.lower(),
            "CTest must be available behind BUILD_TESTING")
    require("add_test(" in cmake.lower(), "CTest suite must register tests")

    for term in ("read-only", "bounded", "safety boundaries"):
        require(term in architecture, f"docs/ARCHITECTURE.md must retain '{term}'")
    for term in ("66 rules", "default to deny", "human", "rollback", "exact pr head"):
        require(term in contract, f"architecture contract must include '{term}'")
    require("human maintainer authorizes merge" in evolution,
            "recursive evolution policy must retain human-controlled promotion")

    print("architecture contract: PASS (C11 core, CTest, policy, bounded/read-only scope)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
