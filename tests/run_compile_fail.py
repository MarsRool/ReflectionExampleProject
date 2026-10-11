#!/usr/bin/env python3
"""Ensure intentionally invalid reflection declarations fail with expected diagnostics."""

import argparse
import subprocess
import sys
from pathlib import Path

TESTS = {
    "unregistered_base.cpp": "base class does not have static property map",
    "invalid_property_name.cpp": "missing",
    "malformed_property_names.cpp": "unexpected empty propertyNames",
    "mismatched_registration.cpp": "property names count doesn't match value pointers count",
}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cxx", default="g++", help="GCC/Clang-compatible compiler")
    parser.add_argument("--repo", type=Path, default=Path(__file__).resolve().parent.parent)
    args = parser.parse_args()

    root = args.repo.resolve()
    include = root / "reflection" / "include"
    test_dir = root / "tests" / "compile_fail"
    if not (include / "reflection" / "reflection.h").is_file():
        parser.error(f"Missing reflection headers at {include}")

    failures = 0
    for filename, expected in TESTS.items():
        source = test_dir / filename
        command = [args.cxx, "-std=c++17", "-fsyntax-only", f"-I{include}", str(source)]
        result = subprocess.run(command, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        # Verify the intended diagnostic, not just an unrelated compilation error.
        success = result.returncode != 0 and expected in result.stderr
        print(f"{'PASS' if success else 'FAIL'}: {filename}")
        if not success:
            failures += 1
            print(result.stderr, file=sys.stderr)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
