#!/usr/bin/env python3
"""Run the C11 production WAV-loader fuzz-smoke CTest target."""
from __future__ import annotations

import argparse
import subprocess
from pathlib import Path

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build", type=Path, default=Path("build"))
    args = parser.parse_args()
    build = args.build.resolve()
    if not (build / "CTestTestfile.cmake").is_file():
        raise SystemExit("Build directory is missing; configure and compile Pytyliki before fuzz-smoke.")
    result = subprocess.run(
        ["ctest", "--test-dir", str(build), "-R", "wav-container-boundary-tests", "--output-on-failure"],
        check=False,
    )
    return result.returncode

if __name__ == "__main__":
    raise SystemExit(main())
