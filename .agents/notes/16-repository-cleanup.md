# Repository cleanup — September 9, 2026

Tor requested a clearer split between reader-facing explanations and agent
working material, a shorter README, and folders containing each trick's source
and `README.md`. During the cleanup he added:

> I’d also probably group all of the things that use gcc’s memoization trick into one folder or category somehow. They’re too spread out right now. The readme is just simply way too wordy and long up front

## Decisions

- Keep root `AGENTS.md` as the discoverable cross-tool entry point. The
  [AGENTS.md convention](https://agents.md/) documents root and scoped instruction
  files. `.agents/notes/` and `.agents/PAPERCUTS.md` are this repository's storage
  convention, not a claim of standard automatic note loading. There are no new
  skills, agent-specific configs, or dependencies.
- Keep the root README to one tagline followed immediately by the catalog.
  Compiler matrices, commands, caveats, and explanations live on the trick pages.
  The standalone-trick / roughly-Turing split and collaboration credit remain.
- Every selected trick has a `README.md` beside its standalone source. The older
  technical sections of `NOTES.md` became these pages; the two existing detailed
  walkthroughs remain intact, with their unique technical evidence merged in.
  Shared attribution lives in `docs/PROVENANCE.md`. There is no second monolithic
  technical catalog to keep in sync.
- GCC memoization is one family with a landing page and four sibling
  constructions: counter, memory, vector, and sortable vector. Put Seance's two
  cache experiments there too; the independent runtime Seance remains a separate
  trick and links to the crossover. Keep the tiny vector separate from the larger
  codec/iterator application within the family.
- `docs/MACHINES.md` keeps the machine status; `docs/Pedantics.md` preserves Tor's
  verbatim criteria. These are useful to readers as well as agents.
- Research notes retain their numbered history in `.agents/notes/`. Its index
  now gives current status and links instead of repeating every session summary.
  Control sources and generator scripts live in the relevant trick's
  `experiments/`. Future applications can use named subfolders with READMEs;
  do not resurrect a miscellaneous root `advanced/` bucket.

## Locations

| Previous location | Current location |
| --- | --- |
| Root spell `x.cpp` | `tricks/x/x.cpp`, except the grouped cache family below |
| `chromatic-aberration.md`, `seance.md` | Each trick's `README.md` |
| `cache-counter.cpp` | `tricks/gcc-memoization/counter/counter.cpp` |
| `cache-memory.cpp` | `tricks/gcc-memoization/memory/memory.cpp` |
| `cache-vector.cpp` | `tricks/gcc-memoization/vector/vector.cpp` |
| `advanced/cache-vector.cpp` | `tricks/gcc-memoization/sortable-vector/sortable-vector.cpp` |
| `NOTES.md` | Sections distributed to the appropriate trick README; shared material in `docs/PROVENANCE.md` |
| `MACHINES.md`, `working-notes/Pedantics.md` | `docs/MACHINES.md`, `docs/Pedantics.md` |
| `working-notes/*.md` | `.agents/notes/*.md`, except Pedantics |
| `PAPERCUTS.md` | `.agents/PAPERCUTS.md` |
| `working-notes/{subsumption-controls.cpp,false-idols-check.py}` | `tricks/false-idols/experiments/` |
| `working-notes/layout-*` | `tricks/chromatic-aberration/experiments/` |
| `working-notes/seance-controls.cpp` | `tricks/seance/experiments/` |
| `working-notes/seance-cache*.cpp` | `tricks/gcc-memoization/experiments/` |

Links and runnable repository commands in earlier notes are updated. Historical
scratch filenames, source hashes, statements about the then-root sources, and
verbatim user messages remain historical evidence. A bare reference to the old
`NOTES.md` in a session narrative describes the documentation at that time.

## Verification

The cleanup changes organization and documentation, not C++ behavior. The only
source edit is Copy gate's comment pointing from `NOTES.md` to its local README.
Verification checks source identity, user quotations, local links and anchors,
repository-relative commands and includes, and relocated examples/controls.
Results:

- All 19 C++/Python files match their original content exactly, allowing only the
  one Copy gate comment change. All 28 existing quoted user-message lines remain
  verbatim. The initial link audit checked 154 local links/anchors and 32
  reproduction commands, plus local C++ includes, with no errors.
- Ubuntu GCC 13.3.0 compiled 15 translation units at `-O2 -Wall -Wextra
  -pedantic-errors`: all relocated examples and controls except the two noted
  below. False idols, Chromatic aberration, and runtime Seance use C++20; the
  others use C++23. The sortable vector additionally uses
  `-fconstexpr-cache-depth=64`. Each file was compiled independently.
- Runtime Seance prints exactly `boo` once. Its control executable exits zero
  and reports `seance controls: six ghosts, zero calls`.
- Both relocated Python cross-check commands pass: 389 SAT cases and 1,159 graphs
  (all member offsets and both class sizes).
- An extra local attempt with the byte-identical one-pointer vector fails on
  GCC 13.3: the evaluator rejects uses of local `v` and loop variable `i` in
  constant expressions. Its documented positive results remain GCC 16.2 and
  Clang 22.1.0. This is outside its advertised compiler configurations, not a
  source change; the limitation is now explicit on its README.
- Empty bits was not recompiled: its positive evidence is Clang-specific and its
  source is byte-identical. No new GCC 16.2 or Clang runs were made in this cleanup.
  Historical cross-compiler evidence is retained as such.

The root README is 33 lines, down from 130, with one GCC memoization catalog row.
No new build system, test framework, or dependencies were added.
