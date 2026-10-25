#!/usr/bin/env python3
"""Run the full Marseflection CMake test suite in strict C++17 mode."""

import argparse
import json
import os
import platform
import re
import subprocess
import sys
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
REPORTS = ROOT / ".ci-reports"
BUILD_ROOT = ROOT / ".ci-build"
CONFIGS = ("Debug", "Release")


def read_benchmark_results(build_dir, config):
    result_dir = build_dir / "benchmarks" / "results" / config
    results = []
    for path in sorted(result_dir.glob("*.json")):
        try:
            results.append(json.loads(path.read_text(encoding="utf-8")))
        except (ValueError, OSError) as exc:
            print(f"Could not read benchmark result {path}: {exc}", file=sys.stderr)
    return results


def benchmark_summary(results):
    if not results:
        return []

    lines = [
        "## Reflection compile-time benchmarks",
        "",
        "Elapsed build time in seconds, including CMake/build-tool startup. "
        "These are informational measurements, not CI performance thresholds.",
        "",
        "| Configuration | Properties | Intrusive | Non-intrusive |",
        "|---|---:|---:|---:|",
    ]
    indexed = {(r["config"], r["count"], r["mode"]): r for r in results}
    config_order = {config: i for i, config in enumerate(CONFIGS)}
    cases = sorted({(r["config"], r["count"]) for r in results},
                   key=lambda item: (config_order.get(item[0], len(CONFIGS)), item[1]))
    for config, count in cases:
        def format_seconds(mode):
            value = indexed.get((config, count, mode))
            if value is None:
                return "n/a"
            if not value["success"]:
                return "FAIL"
            return f"{value['seconds']:.3f} s"

        lines.append(f"| {config} | {count} | {format_seconds('intrusive')} | "
                     f"{format_seconds('nonintrusive')} |")
    return lines + [""]


def run(*args):
    print("\n$ " + " ".join(map(str, args)), flush=True)
    return subprocess.run(args, cwd=ROOT, check=False).returncode == 0


def read_compiler_details(build_dir):
    matches = list(build_dir.glob("CMakeFiles/*/CMakeCXXCompiler.cmake"))
    if not matches:
        return None
    data = matches[0].read_text(encoding="utf-8", errors="replace")
    details = {}
    for key in ("CMAKE_CXX_COMPILER_ID", "CMAKE_CXX_COMPILER_VERSION", "CMAKE_CXX_COMPILER"):
        match = re.search(r'^set\(' + key + r' "([^"]+)"\)', data, re.MULTILINE)
        if match:
            details[key] = match.group(1)
    if details:
        return f"{details.get('CMAKE_CXX_COMPILER_ID', '?')} {details.get('CMAKE_CXX_COMPILER_VERSION', '?')} ({details.get('CMAKE_CXX_COMPILER', '?')})"
    return None


def write_summary(rows, compiler, label, matrix_id, benchmark_results):
    REPORTS.mkdir(exist_ok=True)
    sha = os.environ.get("GITHUB_SHA", "local")
    lines = [
        f"# Compiler compatibility: {label}",
        "",
        f"- Commit: `{sha}`",
        f"- Platform: `{platform.platform()}`",
        f"- Compiler: `{compiler or 'Not detected'}`",
        "- Language: strict C++17 (`CXX_EXTENSIONS=OFF`; pedantic-errors or MSVC /permissive-)",
        "",
        "| Configuration | Check | Status |",
        "|---|---|---|",
    ]
    for config, check, passed in rows:
        lines.append(f"| {config} | {check} | {'PASS' if passed else 'FAIL'} |")
    lines += ["", "Status: **" + ("PASS" if all(row[2] for row in rows) and rows else "FAIL") + "**", ""]
    lines += benchmark_summary(benchmark_results)
    content = "\n".join(lines)
    (REPORTS / (matrix_id + ".md")).write_text(content, encoding="utf-8")
    print("\n" + content, flush=True)
    step_summary = os.environ.get("GITHUB_STEP_SUMMARY")
    if step_summary:
        with open(step_summary, "a", encoding="utf-8") as out:
            out.write(content + "\n")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--generator", default="Ninja")
    args = parser.parse_args()

    matrix_id = os.environ.get("MARSEFLECTION_CI_ID", "local")
    label = os.environ.get("MARSEFLECTION_CI_LABEL", matrix_id)
    msvc = args.generator.startswith("Visual Studio")
    flags = "/permissive- /Zc:__cplusplus /Zc:preprocessor /EHsc" if msvc else "-pedantic-errors"
    base_args = [
        "-DCMAKE_CXX_STANDARD:STRING=17",
        "-DCMAKE_CXX_STANDARD_REQUIRED:BOOL=ON",
        "-DCMAKE_CXX_EXTENSIONS:BOOL=OFF",
        f"-DCMAKE_CXX_FLAGS:STRING={flags}",
    ]
    generator_args = ["-G", args.generator]
    if msvc:
        generator_args += ["-A", "x64"]

    rows = []
    benchmark_results = []
    compiler = None
    suites = [
        ("tests", "tests", ["-DBUILD_TESTING:BOOL=ON"], True),
        ("reflection headers", "reflection", ["-DREFLECTION_BUILD_HEADER_CHECK:BOOL=ON"], False),
        ("extensions headers", "extensions", ["-DEXTENSIONS_BUILD_HEADER_CHECK:BOOL=ON"], False),
    ]
    for config in CONFIGS:
        for name, source, extra, has_ctest in suites:
            build_dir = BUILD_ROOT / matrix_id / config / source
            configure = [
                "cmake", "-S", str(ROOT / source), "-B", str(build_dir),
                *generator_args, *base_args, *extra,
                f"-DCMAKE_BUILD_TYPE:STRING={config}",
            ]
            if not run(*configure):
                rows.append((config, name + " configure", False))
                continue

            if compiler is None:
                compiler = read_compiler_details(build_dir)
            built = run("cmake", "--build", str(build_dir), "--config", config, "--parallel", "2")
            rows.append((config, name + " build", built))
            if has_ctest and built:
                tested = run(
                    "ctest", "--test-dir", str(build_dir),
                    "-C", config, "--output-on-failure",
                    "--no-tests=error", "--parallel", "2",
                )
                rows.append((config, "all CTest tests (incl. compile-fail + single-header + benchmarks)", tested))
                benchmark_results.extend(read_benchmark_results(build_dir, config))

    write_summary(rows, compiler, label, matrix_id, benchmark_results)
    return 0 if rows and all(row[2] for row in rows) else 1


if __name__ == "__main__":
    sys.exit(main())
