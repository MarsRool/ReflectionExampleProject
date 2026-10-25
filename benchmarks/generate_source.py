#!/usr/bin/env python3
"""Generate a single C++17 translation unit to benchmark reflection compilation."""

import argparse
from pathlib import Path


def generate(mode: str, count: int) -> str:
    fields = [f"property_{index:04d}" for index in range(count)]
    lines = [
        '#include "reflection/reflection.h"',
        "",
        "namespace reflection_compile_benchmark",
        "{",
        "struct Record",
        "{",
    ]

    if mode == "intrusive":
        lines.append("    REFLECTION_BEGIN(Record)")
        lines.extend(f"    REFLECTION_PROPERTY(int, {name})" for name in fields)
        lines.append("    REFLECTION_END()")
    else:
        lines.extend(f"    int {name}{{}};" for name in fields)

    lines.extend(["};", ""])

    if mode == "nonintrusive":
        lines.append("REFLECTION_NONINTRUSIVE(Record,")
        for index, name in enumerate(fields):
            suffix = "," if index + 1 < len(fields) else ""
            lines.append(f"    &Record::{name}{suffix}")
        lines.append(")")
        lines.append("")

    lines.extend([
        "using Meta = reflection::StaticPropertyMeta<Record>;",
        f"static_assert(Meta::size() == {count});",
        "",
        "// Force instantiation of traversal and property access, not only registration.",
        "void exerciseReflection(Record& record)",
        "{",
        "    Meta::forEach([&](auto entry)",
        "    {",
        "        (void)decltype(entry)::value.get(record);",
        "    });",
        "}",
        "",
        "} // namespace reflection_compile_benchmark",
        "",
    ])
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--mode", choices=("intrusive", "nonintrusive"), required=True)
    parser.add_argument("--count", type=int, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    if not 1 <= args.count <= 500:
        parser.error("count must be between 1 and 500")

    content = generate(args.mode, args.count)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    if not args.output.exists() or args.output.read_text(encoding="utf-8") != content:
        args.output.write_text(content, encoding="utf-8", newline="\n")


if __name__ == "__main__":
    main()
