# Watching GCC's Astral heap

This is a trace from a **patched and locally built GCC 13.3.0**, not a simulation
of how we expect GCC to work. Open [report.html](report.html) in a browser and
step through the captured allocation, store, read, and deletion events.

## What the capture shows

The [observation source](trace-example.cpp) allocates eight four-EiB objects,
writes two bytes in each, creates two far pointers, reads an untouched midpoint,
and releases the objects. Its `static_assert` passes.

- Eight live allocations total **36,893,488,147,419,103,232 logical bytes**:
  exactly 2^65. Each target byte pointer still has `sizeof == 8`.
- Each allocation has its own artificial `VAR_DECL` and declaration identity.
  In this capture, `p` names allocation 2596 and `q` names allocation 2592.
- Their values are `POINTER_PLUS_EXPR` trees. Both have byte offset
  `4611686018427387903`; their address-expression branches name different
  allocation declarations. The two expressions even reference the same host
  `INTEGER_CST` node for that equal offset in this run.
- Before deletion, each inner byte array's `CONSTRUCTOR` has exactly **two
  explicit entries**: index zero and index `4611686018427387903`. There are also
  outer array/record wrapper constructors; the sixteen-byte-entry count does
  not count all compiler metadata, wrapper entries, or allocated host memory.
- The midpoint lookup reports **no explicit entry**. The byte constructor has
  `CONSTRUCTOR_NO_CLEARING == false`, so the evaluator follows its implicit
  initialization path. The captured binding of `untouched` is integer zero.
- Deletion marks the allocation as deleted and removes its evaluator value.
  The declaration can still appear in the evaluation's allocation list. This
  says nothing about when the host allocator or GCC's collector reclaims RAM.

GCC never packs these logical positions into one flat 65-bit machine address.
The allocation identity and offset reside in ordinary compiler data structures.
The astronomical array bound supplies a logical range; the value representation
stores explicit indexed entries and initialization state.

The upstream implementation being observed is
[GCC 13.3 constexpr.cc](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/gcc/cp/constexpr.cc),
especially `constexpr_global_ctx`, allocation handling in
`cxx_eval_call_expression`, `cxx_eval_store_expression`, and
`cxx_eval_array_reference`. The indexed `CONSTRUCTOR` representation is documented
in [tree.def](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/gcc/tree.def).

## Files and trace boundaries

| File | Purpose |
| --- | --- |
| [gcc-13.3-astral-trace.patch](gcc-13.3-astral-trace.patch) | Release-specific observer patch for `gcc/cp/constexpr.cc` |
| [build-gcc.sh](build-gcc.sh) | Reproduction recipe; builds the compiler front ends, without installing them |
| [trace-example.jsonl.gz](trace-example.jsonl.gz) | Complete compressed stderr capture, including all 145 events |
| [capture.json](capture.json) | Compiler/target/flags and source, patch, archive, and trace hashes |
| [verification.json](verification.json) | Actual compile checks with the observer disabled/enabled |
| [viewer-verification.json](viewer-verification.json) | Browser interaction and narrow-layout checks |
| [render-trace.py](render-trace.py) | Reads the captured data and generates the browser report |
| [viewer.fragment.html](viewer.fragment.html) | Interactive report template |

The report retains 46 events relevant to the byte heap and selected variables.
It filters loop bookkeeping and reads of the small `worlds[]` pointer array;
the complete capture preserves those. It opens at the binding of `q`, when both
pointer trees are available and all eight arrays have their endpoint entries.
Subsequent displayed pointers are the last captured values; after deletion they
are explicitly labeled as referring to a deleted allocation.

The renderer deliberately recognizes the observation example's `bytes` member
and pointer-tree shape. It is not a universal GCC heap decoder. For other
sources, inspect the raw event trees first. It asserts rather than silently
inventing summaries for unsupported shapes or merging separate evaluations.

Trace records preserve tree-code names, host node addresses, `DECL_UID`,
constructor flags, entry counts, explicit indices/values, and expression
operands. Host node addresses are **diagnostic identities of compiler objects**,
not pointer bits belonging to the compiled C++ program. They vary between runs;
declaration IDs and evaluation IDs are also not a stable public GCC interface.
Sizes and offsets are decimal strings so JavaScript cannot round them.

The observer caps constructor entries at 64, expression operands at four, and
recursion depth at twelve; it marks truncation. The selected capture has no
truncated tree nodes. This is not a complete evaluator execution log: the hooks
cover the recorded heap events and value bindings, not every language operation.
The read event distinguishes explicit lookup from the fallback path; it is not
by itself proof that fallback evaluation later succeeded. The successful
compilation and captured `untouched` binding establish that result here.

## Reproduce

Run from this repository's root on a Linux system with a C/C++ compiler, make,
curl, tar/xz/bzip2/gzip, patch, Python 3, and ripgrep. The official source release
contains the generated parser files needed for this build. The recipe downloads
GMP, MPFR, and MPC and verifies their SHA512 hashes using GCC's release helper.
Allow several gigabytes of disk and sufficient RAM for the selected build jobs.

```sh
bash tricks/astral-heap/instrumentation/build-gcc.sh /tmp/astral-gcc

GCC_ASTRAL_TRACE=1 /tmp/astral-gcc/build/gcc/cc1plus \
  -quiet -std=c++20 -O0 -fconstexpr-cache-depth=0 -fsyntax-only -o /dev/null \
  tricks/astral-heap/instrumentation/trace-example.cpp 2> /tmp/astral-trace.log

python3 tricks/astral-heap/instrumentation/render-trace.py \
  /tmp/astral-trace.log /tmp/astral-report.html
```

Set `TRACE_JOBS=4` before the build command if fewer parallel jobs are preferable.
Build logs are in the chosen build directory. No target standard library is
needed for this header-free example. The observer is enabled by the **presence**
of `GCC_ASTRAL_TRACE`; unset it to disable tracing. Output goes to stderr with
one `ASTRAL `-prefixed JSON object per line. Inspect diagnostics and the compiler
exit status before treating output as a successful evaluation.

To regenerate the existing report without building GCC:

```sh
python3 tricks/astral-heap/instrumentation/render-trace.py \
  tricks/astral-heap/instrumentation/trace-example.jsonl.gz /tmp/astral-report.html
```

The local source build used eight jobs, `--disable-bootstrap`, C/C++ only,
checking=release, and `-O0 -g0` for the compiler itself. The observer-disabled
compiler accepts the original heap spell, its controls, the tagged-pointer
construction, and the observation source. Enabling the trace preserves the
observation source's acceptance and produces 145 trace lines with no other
diagnostics. No constexpr resource limits were increased. The observer changes
host timing and memory use, so it should not be used to benchmark stock GCC.

The generated report was checked in headless Chromium: the slider and previous/
next controls update the heap totals, with no JavaScript errors. Desktop and
mobile screenshots were inspected, and layout bounds checked at 320, 390, and
850 pixels wide.

The build recipe was assembled from the commands used in this session and
syntax-checked; the entire script was not rerun from a second empty directory.
The actual patch was compiled and used for the saved capture. This is a research
patch for GCC 13.3, not an upstream GCC option or a stable plugin interface.
No compiler binary is committed. GCC's upstream licensing applies to the
patched GCC source; see its `COPYING3` and source copyright notices.

[Research handoff](../../../.agents/notes/21-astral-instrumentation.md).
