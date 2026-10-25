#!/usr/bin/env python3
"""Compile one isolated CMake target and record elapsed build time."""

import argparse
import json
import subprocess
import sys
import uuid
from pathlib import Path
from time import perf_counter


TOKEN_PREFIX = "// BENCHMARK_BUILD_TOKEN: "


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--config", required=True)
    parser.add_argument("--target", required=True)
    parser.add_argument("--source", type=Path, required=True)
    parser.add_argument("--result", type=Path, required=True)
    parser.add_argument("--mode", choices=("intrusive", "nonintrusive"), required=True)
    parser.add_argument("--count", type=int, required=True)
    args = parser.parse_args()

    # Make every CTest run compile a fresh object, including on coarse-timestamp
    # filesystems. Only mutate a generated file inside the build directory.
    source = args.source.read_text(encoding="utf-8").split(TOKEN_PREFIX, 1)[0]
    args.source.write_text(
        source + TOKEN_PREFIX + uuid.uuid4().hex + "\n",
        encoding="utf-8", newline="\n",
    )

    command = [
        "cmake", "--build", str(args.build_dir),
        "--config", args.config,
        "--target", args.target,
        "--parallel", "1",
    ]
    start = perf_counter()
    result = subprocess.run(
        command,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
        errors="replace",
        check=False,
    )
    elapsed = perf_counter() - start

    args.result.parent.mkdir(parents=True, exist_ok=True)
    args.result.write_text(json.dumps({
        "mode": args.mode,
        "count": args.count,
        "config": args.config,
        "seconds": round(elapsed, 4),
        "success": result.returncode == 0,
    }, indent=2) + "\n", encoding="utf-8")

    print(f"{args.mode:12} {args.count:4} properties "
          f"{args.config:7} {elapsed:.3f} s: "
          f"{'PASS' if result.returncode == 0 else 'FAIL'}", flush=True)
    if result.returncode != 0:
        print(result.stdout, file=sys.stderr)
    return result.returncode


if __name__ == "__main__":
    sys.exit(main())
