#!/usr/bin/env python3
"""Generate a standalone C++17 reflection header from the modular headers."""

import argparse
import os
import re
import sys
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parent.parent
INCLUDE_DIR = REPO_ROOT / "reflection" / "include"
ENTRY = INCLUDE_DIR / "reflection" / "reflection.h"
OUTPUT = REPO_ROOT / "reflection" / "single_include" / "reflection" / "reflection.h"
LICENSE = REPO_ROOT / "LICENSE"

INCLUDE_RE = re.compile(r'^\s*#\s*include\s*([<"])([^>"]+)[>"]')
PRAGMA_ONCE_RE = re.compile(r'^\s*#\s*pragma\s+once\b')
CONDITIONAL_RE = re.compile(r'^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b')
NAMESPACE_OPEN_RE = re.compile(
    r'(?m)^[ \t]*namespace[ \t]+reflection[ \t]*\n[ \t]*\{[ \t]*(?://[^\n]*)?(?:\n|$)'
    r'|^[ \t]*namespace[ \t]+reflection[ \t]*\{[ \t]*(?://[^\n]*)?(?:\n|$)'
)
NAMESPACE_CLOSE_RE = re.compile(
    r'(?m)^[ \t]*\}[ \t]*(?://[ \t]*namespace[ \t]+reflection[ \t]*)?(?:\n|$)'
)
HEADER_SUFFIXES = {".h", ".hh", ".hpp", ".hxx"}


class AmalgamationError(Exception):
    """An input header cannot be merged safely."""


def read_text(path: Path) -> str:
    try:
        # newline=None normalizes CRLF/CR to LF on every platform.
        with path.open("r", encoding="utf-8-sig", newline=None) as source:
            return source.read()
    except (OSError, UnicodeError) as exc:
        raise AmalgamationError(f"Cannot read {path}: {exc}") from exc


def relative_name(path: Path) -> str:
    return path.relative_to(INCLUDE_DIR).as_posix()


def check_exact_case(path: Path) -> None:
    """Reject differently cased paths even on case-insensitive filesystems."""
    relative = path.relative_to(INCLUDE_DIR)
    parent = INCLUDE_DIR
    for part in relative.parts:
        if part not in {item.name for item in parent.iterdir()}:
            raise AmalgamationError(f"Header path has incorrect letter case: {relative.as_posix()}")
        parent /= part


def resolve_include(source: Path, name: str, line_number: int) -> Path:
    # Accept both project-root includes and includes relative to the current file.
    for unnormalized in (INCLUDE_DIR / name, source.parent / name):
        candidate = Path(os.path.normpath(unnormalized))
        if not candidate.is_file():
            continue
        path = candidate.resolve()
        try:
            path.relative_to(INCLUDE_DIR)
        except ValueError as exc:
            raise AmalgamationError(
                f"{relative_name(source)}:{line_number}: Include escapes {INCLUDE_DIR}: {name}"
            ) from exc
        check_exact_case(candidate)
        return path

    raise AmalgamationError(
        f'{relative_name(source)}:{line_number}: Header not found: "{name}"'
    )


def only_trivia(text: str) -> bool:
    """Recognize whitespace and comments outside the reflection namespace."""
    without_comments = re.sub(r'/\*.*?\*/|//[^\n]*', '', text, flags=re.DOTALL)
    return not without_comments.strip()


def unwrap_reflection_namespace(text: str, name: str) -> str:
    """Remove a header's single, outer namespace reflection wrapper.

    This deliberately supports the project's header layout, not arbitrary C++.
    Unexpected top-level content fails instead of silently changing its scope.
    """
    openings = list(NAMESPACE_OPEN_RE.finditer(text))
    if len(openings) != 1:
        raise AmalgamationError(
            f"{name}: Expected exactly one outer 'namespace reflection' block "
            f"(found {len(openings)})"
        )

    opening = openings[0]
    closing_candidates = [m for m in NAMESPACE_CLOSE_RE.finditer(text, opening.end())
                          if only_trivia(text[m.end():])]
    if not closing_candidates:
        raise AmalgamationError(f"{name}: Cannot find the closing 'namespace reflection' brace")
    closing = closing_candidates[-1]

    if not only_trivia(text[:opening.start()]):
        raise AmalgamationError(
            f"{name}: Declarations or directives outside 'namespace reflection' "
            "are not supported"
        )
    # Preserve any leading/trailing comments, including per-header attribution.
    return text[:opening.start()] + text[opening.end():closing.start()] + text[closing.end():]


def heading(name: str) -> str:
    return f"// {'=' * 72}\n// {name}\n// {'=' * 72}\n"


def generate() -> tuple[str, list[str], list[str], list[str]]:
    if not ENTRY.is_file():
        raise AmalgamationError(f"Entry header not found: {ENTRY}")
    if not LICENSE.is_file():
        raise AmalgamationError(f"License not found: {LICENSE}")

    included: set[Path] = set()
    ordered: list[str] = []
    external_includes: list[str] = []
    seen_external_includes: set[str] = set()
    internal_sections: list[str] = []
    entry_section = ""

    def expand(path: Path) -> None:
        nonlocal entry_section
        path = path.resolve()
        if path in included:
            return
        included.add(path)

        name = relative_name(path)
        output_lines: list[str] = []
        conditionals: list[int] = []
        # All includes must precede declarations. That also makes post-order
        # header emission equivalent to the original recursive expansion.
        reached_code = False
        prologue_text = ""
        for number, line in enumerate(read_text(path).splitlines(keepends=True), 1):
            directive = CONDITIONAL_RE.match(line)
            if directive:
                kind = directive.group(1)
                if kind in ("if", "ifdef", "ifndef"):
                    conditionals.append(number)
                elif kind == "endif":
                    if not conditionals:
                        raise AmalgamationError(f"{name}:{number}: Unmatched #endif")
                    conditionals.pop()

            match = INCLUDE_RE.match(line)
            if match:
                if conditionals:
                    raise AmalgamationError(
                        f"{name}:{number}: Conditional #include cannot be consolidated safely"
                    )
                if reached_code:
                    raise AmalgamationError(
                        f"{name}:{number}: #include after declarations is not supported"
                    )
                if match.group(1) == '"':
                    expand(resolve_include(path, match.group(2), number))
                else:
                    include_name = match.group(2)
                    if include_name not in seen_external_includes:
                        seen_external_includes.add(include_name)
                        external_includes.append(f"#include <{include_name}>")
                continue

            if PRAGMA_ONCE_RE.match(line):
                continue
            if line.lstrip().startswith("#") and not directive:
                # An include below a #define/#pragma could depend on it.
                reached_code = True
            output_lines.append(line)
            # Accumulate comments across lines so a multiline comment before
            # an include is not mistakenly treated as a declaration.
            if not reached_code and not line.lstrip().startswith("#"):
                prologue_text += line
                if not only_trivia(prologue_text):
                    reached_code = True

        if conditionals:
            raise AmalgamationError(
                f"{name}:{conditionals[-1]}: Unclosed preprocessor condition"
            )

        body = "".join(output_lines)
        if path == ENTRY.resolve():
            entry_section = heading(name) + body.strip("\n") + "\n"
        else:
            internal_sections.append(
                heading(name) + unwrap_reflection_namespace(body, name).strip("\n") + "\n"
            )
        ordered.append(name)

    # Post-order DFS preserves the previously chosen dependency-first order.
    expand(ENTRY)
    license_lines = read_text(LICENSE).strip("\n").splitlines()
    license_comment = "\n".join("// " + line if line else "//" for line in license_lines)
    preamble = (
        "// Reflection - single-header distribution\n"
        "// Generated by tools/amalgamate.py. Do not edit manually.\n"
        f"//\n{license_comment}\n\n"
        "#pragma once\n\n"
    )
    include_block = "\n".join(external_includes)
    if include_block:
        include_block += "\n\n"

    merged_namespace = (
        "namespace reflection\n{\n\n"
        + "\n".join(internal_sections).rstrip("\n")
        + "\n\n} // namespace reflection\n\n"
    )
    content = preamble + include_block + merged_namespace + entry_section

    all_headers = {
        path.resolve()
        for path in INCLUDE_DIR.rglob("*")
        if path.is_file() and path.suffix.lower() in HEADER_SUFFIXES
    }
    unused = sorted(relative_name(path) for path in all_headers - included)
    return content, ordered, unused, external_includes


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check", action="store_true",
        help="Check whether the generated header matches the committed file",
    )
    args = parser.parse_args()

    try:
        content, order, unused, external_includes = generate()
        encoded = content.encode("utf-8")

        if args.check:
            if not OUTPUT.is_file() or OUTPUT.read_bytes() != encoded:
                print(f"Out of date: {OUTPUT.relative_to(REPO_ROOT)}", file=sys.stderr)
                return 1
            print(f"Up to date: {OUTPUT.relative_to(REPO_ROOT)}")
        else:
            OUTPUT.parent.mkdir(parents=True, exist_ok=True)
            if not OUTPUT.is_file() or OUTPUT.read_bytes() != encoded:
                OUTPUT.write_bytes(encoded)
                print(f"Generated: {OUTPUT.relative_to(REPO_ROOT)}")
            else:
                print(f"Unchanged: {OUTPUT.relative_to(REPO_ROOT)}")

        print(f"Consolidated {len(external_includes)} external includes.")
        print(f"Included {len(order)} headers (dependency-first order):")
        for index, name in enumerate(order, 1):
            print(f"  {index:2}. {name}")
        if unused:
            print("Warning: headers not reachable from reflection/reflection.h:")
            for name in unused:
                print(f"  - {name}")
        return 0
    except (AmalgamationError, OSError) as exc:
        print(f"Amalgamation error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    sys.exit(main())
