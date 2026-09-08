# Session handoff — start here

This directory preserves the September 6–8, 2026 experiments between Tor Shepherd
and Codex. It is working material for future collaborators. The root
[README](../README.md) and [NOTES](../NOTES.md) remain the reader-facing tour.

**Read [Pedantics.md](Pedantics.md) before assessing computational power.** It
preserves Tor's instructions verbatim: roughly-Turing requires recursion or
iteration, memory, and selection/conditional logic. It also covers isolation,
allowed operations, and the distinction between internal value extraction and
observing the final result.

**Latest spell:** [False idols](12-false-idols.md) makes constraint
subsumption decide SAT even though every atomic constraint is literally true.
The standalone example has no function bodies; a 389-case cross-check passes GCC
and Clang. Tor accepted the finding; the source and reader-facing explanation are
now in the root collection. It is not a roughly-Turing claim. The notes preserve
Tor's broadened research targets verbatim, and the
[research directions](13-research-directions.md) preserve the next experiments
and boundaries for future sessions.

[Last rites](11-last-rites.md) uses temporary destruction as the
reverse traversal for automatic differentiation. It records the standalone
spell, the full-expression boundary, GCC checks, and an unresolved Clang
constant-evaluator disagreement. Tor explicitly requested a direction away
from the copy/template-argument work; that investigation was left alone.

[Copy gate](09-copy-gate.md) started the preceding research direction:
changing copy constructors of class-valued template arguments turn overload
viability into a test for normalization. It records the standalone sorting gate,
copy-driven factorial deduction, compiler controls, prior art, and the relevant
standard wording. The cache and pointer investigations were not resumed.

The [equality controls](09-copy-gate.md#equality-controls) establish that this
matching uses structural template-argument equivalence independently of
`operator==`. Tor's requested [“secret equality” follow-up](09-copy-gate.md#follow-up-the-compilers-secret-equality)
is recorded as a separate future research direction.

The [corrected assessment](09-copy-gate.md#roughly-turing-assessment) withdraws
the earlier “yes” as an answer to the isolated-technique question: the
copy-driven factorial's decisions use `if constexpr`. The source still
compiles, but a machine restricted to
“hidden copy plus overload resolution” remains unproven.

[Sortable cache vector](08-sortable-cache-vector.md) preserves
the small root spells and adds an advanced codec/iterator application, with
locally verified mutable sorting. Routine work now goes directly to `main`, as
requested by Tor and recorded in `AGENTS.md`.

[Typed cache values](07-typed-cache.md) tests a generic
byte encoding, an owning-string content encoding, and structured cache keys.
The root dictionary/vector spells still use unsigned 64-bit values.

[Cache memory](06-cache-memory.md) resumed the project with a
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
7. [Typed cache values](07-typed-cache.md): tested encodings beyond integers,
   owning strings, structured keys, and the boundary around arbitrary objects.
8. [Sortable cache vector](08-sortable-cache-vector.md): local access identities,
   deeper cache settings, codecs, proxy swaps, and compile-time sorting.
9. [Copy gate](09-copy-gate.md): template-argument copying, normalization as an
   overload filter, copy-driven recursion, and current-instantiation traps.
10. [Transcription attempt](10-static-antics-transcription.md): local Whisper
    setup worked; the recording could not be downloaded.
11. [Last rites](11-last-rites.md): destruction schedules reverse-mode
    differentiation, with GCC evidence and the Clang lifetime control.
12. [False idols](12-false-idols.md): all-true constraint subsumption as a SAT
    oracle, the reduction, negative controls, and reproducible cross-checks.
13. [Research directions](13-research-directions.md): proposed next experiments,
    success criteria, isolation traps, and other mechanisms considered but not
    tested during the False idols search.

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
topical notes for the later pointer work. The original documentation pass did not
perform new compiler runs; subsequent numbered notes record their own checks.
