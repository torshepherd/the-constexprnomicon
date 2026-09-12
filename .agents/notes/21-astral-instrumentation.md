# Observing GCC's Astral heap

September 12, 2026. Tor asked:

> Hello. I have more questions about astral heap. Do you think we could add debug printouts to GCC itself or some form of output report so that we could build a visualization of how GCC manages to lay out this data internally? I’m really intrigued how it manages to represent an effectively-65-bit address space.

## Investigation and instrumentation

The fresh checkout began at `9ffecd35f8d80516463e29425f8954c44a7e5ede`,
including the concurrent Fine print work. The old session checkout was left
untouched; its staged tree already corresponded to previous app-published work.

Local compiler: Ubuntu GCC 13.3.0. Retrieved the official GCC 13.3.0 release
archive and its pinned constexpr.cc; their file contents matched. The existing
`-fdump-lang-raw` output describes source trees, while `-fdump-tree-original`
was empty for the immediate-only example. Neither supplied the requested live
heap history. No claim that every dump mode on every GCC version is exhausted.

The GCC implementation directly supplies the relevant observation points:

- `constexpr_global_ctx`: owns `hash_map<tree, tree> values`, `heap_vars`, and
  deallocation counters for one evaluation.
- `cxx_eval_call_expression`: intercepts allocation, creates an artificial
  `VAR_DECL`, records its identity, and returns its symbolic address. It also
  recognizes deletion and removes the value.
- `cxx_eval_store_expression`: modifies the corresponding scalar or nested
  `CONSTRUCTOR` value. Trace only after the successful store is complete.
- `cxx_eval_array_reference`: searches explicit constructor indices. Report
  whether lookup hit an explicit entry or proceeds to implicit initialization.

A release-specific patch adds stderr lines prefixed `ASTRAL `, enabled only by
`GCC_ASTRAL_TRACE`. It records an evaluation ID, event, source line, operation
count, subject/value trees, and snapshots of heap VAR_DECLs and their values.
Target sizes/offsets are decimal strings, avoiding JavaScript number rounding.
Tree host addresses are diagnostic node identities, never target pointer bits.
Constructor recursion and entry printing are bounded and mark truncation.

The helper does not allocate C++ program objects, execute extra constexpr
expressions, assign target addresses, or replace the evaluator's storage model.
Printing does change compiler wall time and host resource usage; this is an
observer build, not a benchmark of an unmodified compiler.

## Completed build and capture

Built a non-bootstrapped GCC 13.3 C/C++ compiler with `-O0 -g0`, eight build
jobs, checking=release, and no multilib/NLS/ISL/zstd. Only `all-gcc` is needed
for direct `cc1plus` syntax checks of header-free examples; no target runtime
installation is required. GMP/MPFR/MPC come from the official prerequisite
helper with SHA512 verification. Environment friction is in PAPERCUTS.md.

The patch and observation workload are under
`tricks/astral-heap/instrumentation/`. The patched compiler actually compiled
the workload with C++20, `-O0`, and `-fconstexpr-cache-depth=0`; its static
assertion passed without raising any constexpr resource limit. Target:
`x86_64-pc-linux-gnu`. No compiler binaries are committed.

The initial observer produced 122 events. Adding a `put_value` binding hook
made the local pointer expressions directly visible; rebuilt GCC and captured
the final **145 events** in evaluation 40: 8 allocations, 65 stores, 23 bindings,
32 explicit array reads, 1 implicit array read, 8 before-delete snapshots, and
8 deletions. The compressed JSON-lines file is the complete final capture.

## Findings from actual GCC objects

- The eight independent allocations, IDs 2590 through 2597 in this run, each
  have logical size 2^62. Their maximum live total is exactly 2^65 bytes.
- `p` binds a `POINTER_PLUS_EXPR` whose address branch refers to allocation
  2596. `q` has the same structure but refers to allocation 2592. Both offsets
  are `4611686018427387903`; they reference the very same host `INTEGER_CST`
  node for that equal offset. Neither pointer is represented as one flat
  machine-sized address inside the evaluator.
- Each allocation's storage is an outer array constructor with one element,
  containing a record constructor with the `bytes` field, containing the
  byte-array constructor. Only that inner byte array has two explicit entries:
  indices 0 and 2^62-1. The sixteen-byte-entry total excludes wrapper entries,
  metadata, and other host memory; it is not the compiler's memory footprint.
- The inner constructor's `no_clearing` flag is false. The midpoint index
  `2305843009213693952` has no explicit entry; the evaluator follows implicit
  initialization and binds `untouched` to integer zero. Both the lookup event
  and the resulting binding are captured.
- Deletion removes the evaluator value and marks the allocation declaration
  deleted. Its remaining appearance in `heap_vars` is bookkeeping, not evidence
  of an object that is still alive or a claim about host memory reclamation.
- No tree truncation occurred in the selected capture. Host node addresses
  and declaration/evaluation IDs are run-specific diagnostic identities.

## Verification and report

With tracing disabled, the built compiler accepted the original heap spell,
controls, tagged-pointer payload, and observation source, with no diagnostics.
With tracing enabled, the observation source also passed, producing exactly
145 trace lines and no other diagnostics. `verification.json` records those
checks; `capture.json` records compiler flags and source/patch/archive/raw-log
SHA256 hashes. A Python audit checked allocation/deletion counts, the maximum
live total, the sixteen sparse byte entries, the shared offset node, the zero
binding, and the absence of truncation.

The interactive report is generated from the saved trace, not a simulation.
It selects 46 meaningful events, filtering loop bookkeeping and reads of the
small pointer array; it initially shows the binding of `q`. The complete raw
trace is retained. The decoder is deliberately specific to this observation
source's shape and asserts for unsupported data.

Headless Chromium checked the initial, first, final, previous, and next states
and reported no JavaScript errors. Desktop and mobile screenshots were
inspected; an initially overflowing long allocation-size label was corrected.
The primary controls update the displayed heap and logical-byte totals.
The build recipe reflects the successful session commands and passes Bash
syntax checking; it was not rerun as a second complete clean build.
