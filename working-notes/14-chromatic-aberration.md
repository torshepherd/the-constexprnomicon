# Chromatic aberration — graph coloring by object layout

Research from September 9, 2026, independently assembled in Tor Shepherd's
conversation with Codex. Tor requested another standalone trick, outside the
previous SAT/constraint work. The discovery is an object-layout computation;
it does not use the lifetime store from Empty bits.

**Claim:** On the checked x86-64 GCC/Clang targets, encode each graph edge as an
empty marker type and each vertex as a distinct class inheriting its incident
markers. Laying out the vertices as empty bases or `[[no_unique_address]]`
members computes first-fit graph coloring in declaration order.

The [root spell](../chromatic-aberration.cpp) demonstrates a five-cycle.
[Layout controls](layout-controls.cpp) check order dependence and the effect of
ordinary members. [The graph checker](layout-check.py) compares against a
separate implementation. [The XOR probe](layout-xor.cpp) preserves the finite
circuit interpretation and all four input combinations.

## Encoding and derivation

For a finite simple undirected graph:

1. Give each edge a distinct empty class with alignment one.
2. Give each vertex a distinct empty class, with its incident edge classes as
   direct, public, nonvirtual bases. An isolated vertex is just an empty class.
3. List the vertex classes in the desired coloring order, either as direct
   nonvirtual bases of a graph class or as potentially-overlapping members.

Each vertex's edge bases are distinct types, so they all fit at its offset zero.
Thus two vertices contain same-type subobjects at their origins exactly when
they are adjacent. Those two marker subobjects are separate objects, with neither
nested in the other. Their addresses must differ. Conversely, absent a shared
marker, this encoding supplies no type conflict between the vertices.
See the [object-address rules](https://eel.is/c++draft/intro.object).

The exact choice among permissible offsets comes from the
[Itanium ABI, section 2.4, step II-3](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#non-pod).
An empty component is first tried at zero, then at the current data size with
successive alignment increments on conflict. Placing an empty component leaves
data size unchanged. Here data size starts and stays zero, and alignment is one.

Consequently the color of vertex `v` is:

```text
the least nonnegative integer absent from the colors of v's earlier neighbors
```

That is first-fit coloring. No C++ function computes the missing integer; no
template walks the graph. The compiler's layout pass supplies the search,
collision checks, stored intermediate offsets, and declaration-order traversal.
Future neighbors contribute marker identities to a vertex's type, but cannot
cause placement conflicts until their own subobjects have been placed.

Each individual vertex has size one. The complete graph's size is one greater
than the largest chosen offset, with no alignment rounding beyond bytes. For a
graph with at least one vertex, first-fit uses a contiguous range of colors
starting at zero, so this size is the number of colors it used. A graph with no
vertices has zero colors but an empty C++ class still has size at least one;
that case is excluded from the formula.

For the root five-cycle:

| Vertex | Earlier neighbor colors | Chosen byte offset |
| --- | --- | --- |
| A | none | 0 |
| B | 0 | 1 |
| C | 1 | 0 |
| D | 0 | 1 |
| E | 0, 1 | 2 |

The inherited graph class still satisfies `std::is_empty_v`, but occupies three
bytes. The member-based spelling is standard-layout, so ordinary `offsetof`
provides numeric observations of all five colors as constant expressions.
No object needs to be constructed or pointer representation decoded.

## Essential boundary: this is greedy, not optimal

The four-vertex path `A—B—C—D` needs only two colors.

| Declaration order | Colors in that order | Size |
| --- | --- | --- |
| A, B, C, D | 0, 1, 0, 1 | 2 |
| A, D, B, C | 0, 0, 1, 2 | 3 |

Both are verified in the layout controls. The second order is the useful
counterexample to any claim that `sizeof` returns a graph's chromatic number.
The compiler does not backtrack to improve an earlier placement.

The other control removes `[[no_unique_address]]` from the five-cycle members.
They then occupy five successive bytes on the checked targets; this does not
perform the intended first-fit reuse of colors.

## Finite NAND circuits

The layout rule also supplies a logic interpretation. Regard offset zero as
false and every nonzero offset as true. A new vertex is placed away from zero
exactly when an earlier neighbor is already at zero. Therefore:

```text
truth(new vertex) = NAND(truth(earlier neighbors))
```

Outputs need not be restricted to offsets zero and one. Both one and two are
true. The zero/nonzero interpretation remains closed under subsequent gates:
whether zero is available depends only on the presence of a false predecessor.

Wire a circuit by giving each gate-input pair a fresh edge marker, shared only
by that input vertex and the gate vertex. Put inputs before gates and each gate
after all its inputs. Wires are ordinary inheritance incidences, not reads of
`offsetof` results. Fanout uses separate markers, so consumers do not acquire
unintended edges to each other.

The checked circuit implements XOR with four NAND gates:

```text
t = NAND(x, y)
l = NAND(x, t)
r = NAND(y, t)
out = NAND(l, r)
```

These equations explain the wiring; they are not expressions executed in the
C++ probe. Each namespace in `layout-xor.cpp` spells out the same topology for
one input combination. A zero seed is laid out first. A true input shares an
additional marker with it; a false input does not. The code checks the encoded
inputs, the first NAND, and the final XOR value for all four combinations.

The entire probe contains declarations and final assertions: no function
definitions, constexpr/consteval functions, templates, concepts, overloads,
friend injection, or internal value extraction. Header machinery provides
`offsetof` and the final standard-layout trait only.

This establishes a composable finite Boolean-circuit construction. NAND
composition supports arbitrary finite Boolean circuits, and the preceding
encoding explains how to represent their wires without computing the answers
in a generator. It **does not establish roughly-Turing completeness** under
[Tor's criteria](Pedantics.md). All gates are spelled out in a finite class;
the layout scan is not a demonstrated way to repeat a program transition beyond
those source-declared components. No recurrence mechanism was found or claimed.

## Compiler evidence

All invocations below completed with exit code zero and no diagnostics. Local
compiler: `g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0`. Remote compiler IDs were
discovered through the Compiler Explorer API: `g162` is x86-64 GCC 16.2,
`clang2210` is x86-64 Clang 22.1.0.

| Source/check | Local GCC 13.3 | GCC 16.2 | Clang 22.1.0 |
| --- | --- | --- | --- |
| Root spell | C++20, O0 and O2 | C++20, O2 | C++20, O2 |
| Order and ordinary-member controls | C++20, O0 and O2 | C++20, O2 | C++20, O2 |
| Four XOR input combinations | C++20, O0 and O2 | C++20, O2 | C++20, O2 |
| All 1,159 graph cases | C++20, O0 | Not run | Not run |
| 135-case subset | Covered by full suite | C++20, O2 | C++20, O2 |

All runs used `-Wall -Wextra -pedantic-errors`; local runs used
`-fsyntax-only`. Remote final checks concatenated the root spell, layout
controls, and XOR probe into one translation unit. The remote graph subset
was a separate translation unit. Preliminary five-cycle probes also passed
locally and on Clang, before the final source was assembled.

The full graph suite consists of every labeled simple graph on one through five
vertices (1 + 2 + 8 + 64 + 1024 = 1,099), plus 20 seeded graphs each on 8, 16, and
24 vertices. The smaller suite omits the 1,024 five-vertex graphs, leaving 135.
Python performs an independent greedy algorithm to produce expected assertions.
The C++ declarations encode only the graphs, not their answers. Each case checks
every member offset, the member-based graph's size and standard-layout property,
and the inherited graph's size and empty-class property.

Reproduce from the repository root:

```sh
g++ -std=c++20 -O0 -Wall -Wextra -pedantic-errors -fsyntax-only chromatic-aberration.cpp working-notes/layout-controls.cpp working-notes/layout-xor.cpp
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only chromatic-aberration.cpp working-notes/layout-controls.cpp working-notes/layout-xor.cpp
python3 working-notes/layout-check.py
python3 working-notes/layout-check.py --small --source > /tmp/layout-subset.cpp
```

The last command prepares the smaller source for another compiler; it does not
itself perform a compiler check. The source generator accepts `--compiler` to
use another locally installed compiler.

Remote compilation succeeded, but automatic approval review rejected creation
of a permanent public Godbolt share link as a separate publication action.
No share link is supplied. The checked source and reproduction commands are
preserved in the repository.

## Portability, provenance, and next boundaries

The classes are ordinary well-formed C++. The specific packing, color numbers,
and size relationship rely on the described ABI algorithm; ISO C++ alone does
not require them. The checks cover x86-64 GCC/Clang, not MSVC or other targets.
No undefined behavior, evaluator bug, or mutation of compiler caches is needed.
Changing alignment, adding data, using virtual inheritance, reusing a marker
for unrelated edges, or reusing a vertex type changes the problem being encoded.

The ABI's overlap algorithm is documented prior work. Microsoft's
[empty-base optimization account](https://devblogs.microsoft.com/cppblog/optimizing-the-layout-of-empty-base-classes-in-vs2015-update-2-3/)
also illustrates that empty-class layouts differ between implementations.
Targeted searches for empty-base optimization/no_unique_address plus graph
coloring did not find this graph encoding. That is limited search evidence,
not a historical priority proof. Credit the human/AI collaboration and label
the construction independently derived.

Useful future boundaries, not unfinished promises:

- The finite NAND result is already tested. Do not rediscover it by adding a
  constexpr helper that evaluates gates; that would obscure the primitive.
- A layout-only recurrence claim would need a way to cause further transitions
  without predeclaring every vertex. Ordinary template recursion would enlarge
  the allowed mechanism, not silently establish layout-only recurrence.
- The least-excluded-value rule is also the recurrence for Grundy values in an
  explicitly supplied acyclic impartial-game graph. That interpretation is a
  possible application; no separate game example was compiled in this session.
- Keep the small coloring spell intact. A general graph/circuit library, larger
  generated applications, and an optimization claim are not needed to establish
  this result or requested as the next task.
