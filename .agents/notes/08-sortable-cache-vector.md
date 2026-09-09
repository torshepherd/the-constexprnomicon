# Sortable cache vector — September 7–8, 2026

Tor asked to keep the elegant simple version and save a more capable serializable
version as an advanced application, adding iterators, sortedness checks, and
possibly mutable sorting. During the work he clarified that this repository
should use direct commits to main, not PRs. `AGENTS.md` now records that policy.

The selected [advanced source](../../tricks/gcc-memoization/sortable-vector/sortable-vector.cpp) stands alone. The
original `cache-memory.cpp` and `cache-vector.cpp` are byte-for-byte unchanged.
The README separates small spells from advanced applications, and NOTES.md gives
the reader-facing explanation and usage constraints.

## The two obstacles

1. A default line number is not a fresh call identity in an algorithm loop.
   Saved reads and repeated writes in the original spell can be memoized.
2. Standard algorithms add stack frames. Warming at depth 500 leaves little
   room under the default recursion limit, and the default cache depth of 8
   does not retain writes reached through some deeper algorithm calls.

The new implementation uses an unused local-object address in each read/write
wrapper and keeps the underlying `ember` keys purely integral. Warm/probe depths
are 384/640. The compile command raises **cache depth**, not recursion depth, to
64. Do not confuse these two settings.

## Local identity experiment

An isolated integer-cache test repeatedly writes values 0 through 5 to one key
and reads them back through the same call sites, then checks writes and reads
across separate static assertions. It passed locally on GCC 13.3.0 when wrappers
took an `int&` and incremented it. It also passed with the increment removed,
then with an unused `const void*` pointing to a local object. Replacing that local
address with `nullptr` failed the loop assertion. These runs used C++23 `-O0`
and the default cache/recursion settings, with warm/probe depths 384/640.

Thus an incrementing sequence number was not necessary for this access pattern.
The observation is consistent with GCC not memoizing calls containing these
local-object arguments, while still memoizing their integral-only inner calls.
No complete compiler-source diagnosis was performed. The source deliberately
keeps `(void)anchor`; removing the apparently unused parameter can break it.

The advanced handle is empty and local to the enclosing constant evaluation.
All handles of one specialization address the same cache, so copying the handle
does not snapshot data. Proxies and iterators carry the handle's pointer. Their
lifetimes must stay within that handle's lifetime; do not substitute a global or
null identity and assume equivalent cache behavior.

## Serialization and algorithms

`cache_vector<T, Codec, Tag>` has a per-specialization cache domain. Each encoded
byte gets eight bit facts plus a presence marker. The revision has a separate
completion marker. This permits variable-length payloads, including an empty
encoding, without reserving a byte value. Key −1 stores the vector length.

The default codec uses object bytes for suitable types. `text` serializes string
contents. `record_codec` serializes a padded record field by field and reconstructs
it with a non-default constructor. The general get path does not require `T{}`;
only the absent integer length returns zero, and an unwritten element throws.
No arbitrary pointer/alias/ownership codec is claimed.

The random-access iterator returns a writable proxy. Assignment from another
proxy materializes the source value. ADL swap saves one decoded value before
writing either location. `iter_move` returns a value and `iter_swap` forwards to
the value exchange. Nothing was added to namespace std. Qualified `std::swap`
is not the proxy customization mechanism; use unqualified swap with a using
declaration, `std::iter_swap`, or the ranges customization points.

The final checks include:

- Random-access iterator and sortable constraints for integer/string iterators.
- Unsorted integer initialization in one evaluation, ranges sorting in another,
  and exact persisted values checked in a third.
- A saved proxy that observes a later write in the local-handle access pattern.
- ADL swap, `std::iter_swap`, ranges swap, and ranges iter_swap.
- Traditional descending `std::sort`/`std::is_sorted`, then ascending ranges sort.
- Strings including an 80-character allocation, empty contents, and embedded null.
- Empty/singleton sortedness, clear, pop, and reuse.
- A custom comparator sorting a padded non-default-constructible record through
  its field-wise codec, followed by a separate evaluation checking persistence.
- A separate tagged 20-element integer domain compared with a sorted std::array.
  This exercises libstdc++'s partitioning path beyond its threshold of 16.
- Empty, size-1 handle checks. Iterator/proxy objects are not claimed to be empty.

## Verification and failures

Final source SHA-256:

```text
06af0257411a34d13ba7a750a1f26aa0900b1f3863430c719def13cb78654bc1
```

Compiler: local Ubuntu GCC 13.3.0 (`13.3.0-6ubuntu2~24.04`) with its libstdc++.
Both full-source commands passed with exit 0 and no diagnostics:

```sh
g++ -std=c++23 -O0 -fconstexpr-cache-depth=64 -fmax-errors=2 -c tricks/gcc-memoization/sortable-vector/sortable-vector.cpp -o /tmp/advanced-cache-vector-O0.o
g++ -std=c++23 -O2 -fconstexpr-cache-depth=64 -fmax-errors=2 -c tricks/gcc-memoization/sortable-vector/sortable-vector.cpp -o /tmp/advanced-cache-vector-O2.o
```

The earlier six-integer prototype failed its sortedness/value and persistence
assertions with the default cache depth. Adding `-fconstexpr-cache-depth=64`
made that same prototype pass. The subsequent full source uses that setting.
The recursion limit remains the compiler's default 512. No large-container
performance or portability claim follows from these small experiments.

A Compiler Explorer request for the advanced source was rejected by tool review
because it would disclose new source to Godbolt without explicit upload approval.
It was not retried through another route. All advanced-source verification claims
are local GCC 13.3.0 only; earlier Godbolt results for unchanged root spells do
not verify the advanced version. Ask for approval before attempting that upload.
Some earlier scratch requests were interrupted by limits; do not count a started
request as a confirmed successful compile.

## Repository handoff

Bring the previous draft branch's commits onto main without rewriting history,
publish this advanced application and workflow note directly there, and close
draft PR #1 if it remains open. Keep the branch's historical commits intact.
For subsequent sessions the direct-main rule in AGENTS.md supersedes the earlier
PR-based workflow. The zero-storage owning-vector investigation remains tabled.
