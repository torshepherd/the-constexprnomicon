# Cache memory

[Source](memory.cpp)

The cache counter can remember membership. Give each cached fact a key, a
revision, and a bit position, and it can remember whole values:

```cpp
static_assert(remember(7, 42) == 42);
static_assert(recall(7) == 42);
static_assert(remember(7, 99) == 99);
static_assert(recall(7) == 99);
```

These are separate constant evaluations. No C++ object carries the dictionary
between them; there are no templates, friend injections, or mutable globals in
the spell. Keys and values arrive as ordinary function arguments. The compiler
does spend memory retaining its cache.

`ember(key, revision, bit, 500)` warms one fact. The corresponding 520-deep
probe recognizes a warm fact and fails on a cold one under the tested default
limits. Positions 0–63 encode the set bits of an unsigned 64-bit value.
Position 64 marks a completed revision, including a write of zero.

`remember` finds the first unmarked revision, warms its set bits, then marks it.
`recall` scans the same markers and reconstructs the last completed value.
An unwritten key reads as zero. Overwriting never clears cache entries: the new
revision supersedes the old one. The revision scan grows linearly with the
number of writes to that key. There is no erase or reclamation.

The cache also remembers calls to `remember` and `recall` themselves. Their
unused line-number arguments distinguish the top-level calls in this sketch.
Reusing an identical read tuple can return a stale value; replaying an identical
write tuple can return its old result without writing again. The checks exhibit
both, including a cached miss surviving a later write. Calls from a loop or
wrapper need their own investigation: a source line is not a fresh invocation
ID, and wrapper memoization and extra call depth also matter.

The final checks use successive reads as array bounds: the same key supplies
3 and then 5 to two type aliases. That demonstrates state influencing later
type formation; it does not make an arbitrary function parameter a constant
expression within that function's definition.

The complete source passes GCC 13.3.0 and GCC 16.2 at C++23 `-O0` and `-O2`.
Clang 22.1.0 fails at both levels, reading zero after the first write. GCC 16.2
with caching disabled also fails that read. A bounded primitive control gives
the same 0→1 / 0→0 / 1→1 results as the counter's controls above.

Keep the default constexpr depth and cache settings. Raising the recursion
limit enough to make cold facts recognizable breaks the revision scan; do not
try that control on the unbounded full spell. This is observed evaluator
behavior within one translation unit, not ISO-portable storage or persistence
across compilations. The constants leave only a small margin for extra call
depth. [Working notes](../../../.agents/notes/06-cache-memory.md) preserve the controls
and exact source hash. This is an application of the existing cache counter,
without a claim of historical priority.

## Try it

Recorded compiler evidence: **GCC 13.3 and 16.2, default constexpr depth/cache settings**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++23 -O2 -c tricks/gcc-memoization/memory/memory.cpp -o /tmp/cache-memory.o
```

See [shared provenance](../../../docs/PROVENANCE.md) for known ingredients and related work.
