# GCC memoization

**The compiler's cache is the storage.** Warm a recursive call until a later probe
can finish within GCC's evaluation limit; whether it succeeds becomes a readable
bit. The constructions below turn that one trick into increasingly capable stores.

| Construction | What it adds | Recorded compiler evidence |
| --- | --- | --- |
| [Counter](counter/) | Remember which integer keys have been seen. Start here for the primitive. | GCC 13.3, 16.2 |
| [Memory](memory/) | Encode 64-bit values and overwrite dictionary entries through revisions. | GCC 13.3, 16.2 |
| [Vector](vector/) | Put an empty, const global handle over mutable integer elements. | GCC 13.3, 16.2 |
| [Sortable vector](sortable-vector/) | Add codecs, live proxies, and iterators; sort integers, strings, and records. | GCC 13.3 |

Each construction has its own standalone source and explainer. Use the exact
settings on that page: the small spells use the default constexpr depth/cache
settings; the sortable version needs `-fconstexpr-cache-depth=64`.
Calls can themselves be memoized, so repeated identities and wrapper calls matter.
These results are observed GCC behavior within one translation unit, not portable
C++ storage or persistence across compilations.

## Seance crossover

Can a failed concept trigger a write into this memory? Yes, on the checked GCC
configurations. A direct nested requirement can do it too, without Seance's helper.

- [Bounded cache controls](experiments/seance-cache.cpp) separate instantiation,
  required evaluation, optional folding, and repeated-call identities.
- [Dictionary application](experiments/seance-cache-memory.cpp) writes values
  that later constant evaluations can read.
- [Explanation](../seance/README.md#crossing-over-with-gcc-cache-memory) and
  [compiler evidence](../../.agents/notes/15-seance.md#follow-up-can-the-ghost-write-into-gccs-constexpr-cache).

These are applications of the same storage trick. The
[original Seance](../seance/) is an independent runtime-initialization trick.

## Research history

[Primitive and controls](../../.agents/notes/03-compiler-state.md) ·
[Dictionary and vector](../../.agents/notes/06-cache-memory.md) ·
[Typed encodings](../../.agents/notes/07-typed-cache.md) ·
[Sorting](../../.agents/notes/08-sortable-cache-vector.md)
