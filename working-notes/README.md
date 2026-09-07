# Session handoff — start here

This directory preserves the September 6–7, 2026 experiments between Tor Shepherd
and Codex. It is working material for future collaborators. The root
[README](../README.md) and [NOTES](../NOTES.md) remain the reader-facing tour.

**Latest work:** [Cache memory](06-cache-memory.md) resumes the project with a
writable integer dictionary in GCC's constexpr memoization cache. It records the
new dictionary and singleton vector spells, checks, limitations, and possible
next applications. Documents 01–05
preserve the original session handoff below.

**The zero-storage owning vector was not achieved and is tabled.**
Do not resume that search automatically. No transcription,
compiler-bug report, or scheduled recheck is pending in the background.

## Read in this order

1. [Context and decisions](01-context-and-decisions.md): what Tor wants, how the
   project came about, provenance, and corrections to early excitement.
2. [Span, unions, and vector](02-span-and-vector.md): the starting trick and the
   evolution from an integer container to the small `vector<T>` spell.
3. [Compiler state](03-compiler-state.md): cache counter, empty bits, the
   nested-union byte, recursion probes, and side effects.
4. [Pointer investigation](04-pointer-investigation.md): `empty<T>`, attempted
   pointer serialization, the standard-library-name cast loophole, and why
   these pieces did not produce an empty vector.
5. [Reproduction and handoff](05-reproduction-and-handoff.md): verification history,
   compiler settings, editable links, and how to resume without repeating work.
6. [Cache memory](06-cache-memory.md): extending cached membership to overwritable
   64-bit values and a singleton vector, stale reads and writes, and verification.

## State at the original handoff

| Construction | Observed result | Location |
| --- | --- | --- |
| One-pointer `vector<T>` | GCC 16.2 and Clang 22.1.0, C++23, `-O0`/`-O2` | [Root spell](../one-pointer-vector.cpp) |
| Empty class with 64 writable logical bits | Clang 22.1.0, C++23, `-O0`/`-O2`; GCC rejects | [Root spell](../empty-bits.cpp) |
| Counter in constexpr memoization | GCC 16.2, C++23, `-O0`/`-O2`; Clang fails increment checks | [Root spell](../cache-counter.cpp) |
| Empty scalar wrapper | Tested `int` and `double` on Clang, `-O0`/`-O2` | [Pointer notes](04-pointer-investigation.md) |
| Nested-union one-byte integer | Compile-time assertions pass on both at C++23 `-O2`; ordinary call differs | [Compiler-state notes](03-compiler-state.md#the-nested-union-byte) |
| Cast through fake `std::source_location::current` | Same-type `void*` recovery on both, C++23, `-O0`/`-O2` | [Pointer notes](04-pointer-investigation.md#the-library-name-loophole) |
| Pointer bits → empty storage → original owning pointer | No working construction found | Tabled |

Compiler acceptance here is an observation, not proof of portable ISO C++.
The fake `std` declarations are deliberate nonconforming experiments. Neither
that cast nor GCC's accepted integer round trip yielded readable address bits.

## Using these notes

These are working documents, not a source/result archive. They record the
mechanisms, observed compiler results, unsuccessful approaches, decisions, and
existing Godbolt links. Historical scratch filenames identify probes discussed
in the session; those scratch files are not committed here.

The root sources and their assertions are the selected spells. Use the linked
Godbolt examples for earlier editable sources, and the code and controls in the
topical notes for the later pointer work. No new compiler runs were performed
for this documentation pass.
