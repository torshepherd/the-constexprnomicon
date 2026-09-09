#!/usr/bin/env python3
"""Cross-check class layout against an independent first-fit graph coloring.

Default: check 1,159 graphs with local g++.
--small: 135 graphs (all through four vertices, plus larger random graphs).
--source: print C++ for another compiler instead of invoking the local compiler.
Expected colors occur only in assertions, not in the class declarations.
"""

import argparse
import itertools
import random
import subprocess
import tempfile
from pathlib import Path


def cases(small):
    for n in range(1, 5 if small else 6):
        edges = list(itertools.combinations(range(n), 2))
        for mask in range(1 << len(edges)):
            yield n, [edge for i, edge in enumerate(edges) if mask >> i & 1]
    rng = random.Random(20260909)
    for n in (8, 16, 24):
        for _ in range(20):
            probability = rng.choice([0.1, 0.3, 0.5, 0.8])
            yield n, [edge for edge in itertools.combinations(range(n), 2)
                      if rng.random() < probability]


def emit(index, n, edges):
    lines = [f"namespace case_{index} {{"]
    for a, b in edges:
        lines.append(f"struct e{a}_{b} {{}};")
    for v in range(n):
        bases = [f"e{a}_{b}" for a, b in edges if v in (a, b)]
        inheritance = " : " + ", ".join(bases) if bases else ""
        lines.append(f"struct v{v}{inheritance} {{}};")
    lines.append("struct graph {")
    for v in range(n):
        lines.append(f"[[no_unique_address]] v{v} m{v};")
    lines.append("};")
    lines.append("struct bases : " + ", ".join(f"v{v}" for v in range(n))
                 + " {};")

    colors = []
    for v in range(n):
        forbidden = {colors[a] for a, b in edges if b == v}
        color = next(c for c in range(n) if c not in forbidden)
        colors.append(color)
        lines.append(f"static_assert(offsetof(graph, m{v}) == {color});")
    lines.append(f"static_assert(sizeof(graph) == {max(colors) + 1});")
    lines.append("static_assert(std::is_standard_layout_v<graph>);")
    lines.append(f"static_assert(sizeof(bases) == {max(colors) + 1});")
    lines.append("static_assert(std::is_empty_v<bases>);")
    lines.append("}")
    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--small", action="store_true")
    parser.add_argument("--source", action="store_true")
    parser.add_argument("--compiler", default="g++")
    args = parser.parse_args()
    graphs = list(cases(args.small))
    source = "#include <cstddef>\n#include <type_traits>\n" + "\n".join(
        emit(i, n, edges) for i, (n, edges) in enumerate(graphs)
    ) + "\n"
    if args.source:
        print(source, end="")
        return
    with tempfile.TemporaryDirectory(prefix="chromatic-aberration-") as temp:
        path = Path(temp) / "checks.cpp"
        path.write_text(source)
        subprocess.run([args.compiler, "-std=c++20", "-O0", "-Wall", "-Wextra",
                        "-pedantic-errors", "-fsyntax-only", str(path)], check=True)
    print(f"PASS: {len(graphs)} graphs, all member offsets and both sizes")


if __name__ == "__main__":
    main()
