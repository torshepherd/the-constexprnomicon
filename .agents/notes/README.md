# Research handoff

Working material for collaborators. The [root README](../../README.md) is the
reader's catalog; each trick's own README holds its explanation and limitations.
Read [AGENTS.md](../../AGENTS.md), [Pedantics](../../docs/Pedantics.md), and the
[papercut log](../PAPERCUTS.md), then choose the relevant notes below.

## Current state

- New [Fine print](20-fine-print.md): function declarations run a parentheses
  parser through their exception specifications, then a generic cyclic-tag
  interpreter supplies selection, evolving memory, and recurrence. The explicit
  combination is deduction/overload selection, pack substitution, and lazy
  exception-specification propagation. GCC 13.3/16.2 and Clang 22.1.0 pass the
  C++11 sources and 1,451 generated terminating checks. No function bodies,
  constexpr functions, or intermediate member-value extraction are involved.
- Previous research: [Seance and the GCC cache crossover](15-seance.md). A failed
  concept can trigger a dictionary write, but a direct nested requirement can
  trigger it too: an application of existing storage, not a new primitive.
- New standalone result: [Astral heap](17-astral-heap.md). GCC represents eight
  simultaneous four-EiB objects with eight-byte target pointers, beyond a flat
  64-bit address space. Sparse aggregate initialization and symbolic allocation
  identities supply the mechanism; disabling constexpr caching preserves it.
  September 11 follow-up: a single pointer carries a recoverable 65-bit value
  relative to the shared eight-array codebook. GCC's constantness probe identifies
  the bank; same-array subtraction recovers the offset. Local GCC 13.3 evidence.
  Follow-up removes the builtin with a fixed tagged codebook: read the bank tag,
  cast to its slot type, and subtract within the array. GCC 13.3 and 16.2 pass;
  the authorized Godbolt retry also verifies the original pointer encoding.
- Fresh exhaustive search: [out-of-left-field search](18-out-of-left-field-search.md).
  Initializedness/bit-field storage was rejected as the same evaluator-metadata
  family as Empty bits, but remains preserved, including corrected GCC/Clang
  evidence. Weaker and unsuccessful directions are recorded too.
- [Phase shift follow-up](19-phase-shift.md): one unchanged type has two binding
  elements before its `tuple_size` specialization is completed and three after.
  The standalone spell passes GCC 13.3, GCC 16.2, and Clang 22.1.0 in C++17;
  its self-bootstrap variant and retained controls have local GCC evidence.
  A modest declaration-order curiosity, not a new storage primitive or machine.
- [Machine investigations](../../docs/MACHINES.md) track demonstrated ingredients
  separately from complete machines. Fine print is a demonstrated explicitly
  combined machine; the single-feature investigations remain incomplete. Apply
  Tor's isolation rules.
- [Research directions](13-research-directions.md) and each later note preserve
  untested leads. Choose a bounded investigation rather than treating every lead
  as pending work.
- The zero-storage owning vector remains **tabled**. Do not resume it automatically.
  No transcription, compiler-bug report, or scheduled recheck is pending.
- [Repository cleanup](16-repository-cleanup.md) records the current layout.
  Earlier notes retain historical filenames and statements about the then-root
  collection; links and runnable repository commands now use the new locations.

## Notes by topic and session

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
14. [Chromatic aberration](14-chromatic-aberration.md): graph coloring through
    empty object layout, the ABI derivation, independent graph checks, a greedy
    non-optimality control, and a finite NAND/XOR interpretation.
15. [Seance](15-seance.md): false concepts and rejected overloads leave runtime
    initialization footprints through auto return-type deduction; exact controls
    separate instantiation from execution and record the deferral boundary.
16. [Repository cleanup](16-repository-cleanup.md): mechanism-based folders,
    reader-facing explainers, and updated reproduction commands.
17. [Astral heap](17-astral-heap.md): a 32-EiB logical heap with eight-byte pointers,
    sparse aggregate initialization, the 32-bit counterpart, and rejection controls.
18. [Out-of-left-field search](18-out-of-left-field-search.md): the exhaustive
    post-Astral search, rejected recursive-arrow, appertainment, RTTI,
    exceptions, initializedness, and implicit-object-creation directions, plus
    the unfinished mid-file structured-binding phase shift.
19. [Phase shift](19-phase-shift.md): cross-compiler main-source verification,
    a self-bootstrapping tuple adapter, same-object references, runtime and arity
    controls, failed const/template caching hypotheses, and bounded future leads.
20. [Fine print](20-fine-print.md): bodyless computation in exception
    specifications, a parentheses parser, generic cyclic-tag machine, exact
    isolation boundary, corrected probes, cross-compiler evidence, and search trail.

## State at the original handoff

| Construction | Observed result | Location |
| --- | --- | --- |
| One-pointer `vector<T>` | GCC 16.2 and Clang 22.1.0, C++23, `-O0`/`-O2` | [Source](../../tricks/one-pointer-vector/one-pointer-vector.cpp) |
| Empty class with 64 writable logical bits | Clang 22.1.0, C++23, `-O0`/`-O2`; GCC rejects | [Source](../../tricks/empty-bits/empty-bits.cpp) |
| Counter in constexpr memoization | GCC 16.2, C++23, `-O0`/`-O2`; Clang fails increment checks | [Source](../../tricks/gcc-memoization/counter/counter.cpp) |
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

The sources in `tricks/` and their assertions are the selected spells. Use the
linked Godbolt examples for earlier editable sources, and the code and controls in the
topical notes for the later pointer work. The original documentation pass did not
perform new compiler runs; subsequent numbered notes record their own checks.
