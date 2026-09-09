# Astral heap — more addressable bytes than pointer representations

September 9, 2026. Tor asked for another standalone trick, separate from the
existing union/lifetime storage, cache storage, copy/equality, destructor
gradient, and empty-base graph-coloring work. The fresh checkout also contained
Seance and its cache crossover; those were excluded from the search.

## Selected result

[Astral heap](../../tricks/astral-heap/astral-heap.cpp) creates eight simultaneous
four-EiB objects through `new realm{}`, writes and reads distinct endpoints,
checks untouched zeroes and allocation identities, and frees all objects.
Both the object pointers and byte pointers have eight-byte target representations.
The aggregate live size is 2^65 bytes, greater than a flat 64-bit address space.

The [explainer](../../tricks/astral-heap/README.md) gives the derivation,
implementation references, exact reproduction commands, and limitations. Keep
the claim about logical evaluator storage. It is not physical RAM, runtime
allocation success, a single object with an unrepresentable size, or a pointer
whose runtime representation has secretly grown.

The compiler's sparse initializer representation and symbolic allocation
identities do the unusual work. Ordinary loops only create and inspect the
objects. This is independent of the memoization family and remains accepted with
`-fconstexpr-cache-depth=0`. The only implementation macro in the source checks
the number of bits in a byte; no probing builtin supplies storage or control flow.

## Derivation and discarded routes

- Surveyed implicit special-member deletion and exception-specification
  inference. Compiled unions nested around a class with a nontrivial copy
  constructor, a nontrivial destructor, and deleted copies. Nesting propagated
  unusability rather than producing a composable negation. A move-fallback
  wrapper with a declared noexcept copy and defaulted move produced ordinary
  fallback behavior. Nothing qualified as a new primitive. The noexcept circuit
  ideas were considered but not implemented as an isolated machine.
- A bounded GCC probe of `new unsigned char[1ull << E]` with writes at 0,
  half, and the last element passed at E=16, 28, 40, 60. This established sparse
  uninitialized allocation; no untouched elements were read.
- Adding `{}` directly to that array-new at E=60 hit GCC 13.3's default
  262144-iteration constexpr loop limit. Do not claim all equivalent-looking
  initialization paths are cheap.
- A local `unsigned char p[1ull << 60]{}` with endpoint/middle writes and
  untouched-zero reads passed GCC 13.3 O0 under the same memory cap.
- `struct realm { unsigned char bytes[1ull << 62]; };` plus `new realm{}`
  retained aggregate initialization's sparse zero representation while giving
  each allocation an independent identity. Eight allocations crossed the entire
  target address-space bound. This is the selected source.
- Clang 18.1.3 accepted the uninitialized E=16 array-new probe. At E=28 it
  rejected the allocation against its 1048576-element constexpr limit; at
  E=40/60 it reported that the bound was too large. Its final four-EiB member
  array is rejected at type checking, as is Clang 22.1.0's. No oversized Clang
  allocation was attempted with its protective limits disabled.

## Compiler evidence

Local environment: Ubuntu in WSL on the Windows Codex host. GCC identifies as
`g++ (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0`; Clang as Ubuntu 18.1.3.
Compiler Explorer targets were discovered as `g162` and `clang2210`.

All final checks used `-Wall -Wextra -pedantic-errors`. Local builds were
syntax-only. Each local compiler subprocess inherited `RLIMIT_AS=512 MiB` and
`RLIMIT_CPU=15 seconds`, with a 20-second parent timeout. No successful run
approached those limits. Peak values below are Linux `RUSAGE_CHILDREN.ru_maxrss`
from a fresh wrapper process per compiler command.

| Source and target | Flags beyond warnings | Outcome / peak KiB / elapsed seconds |
| --- | --- | --- |
| Spell, local GCC 13.3 | C++20 O0 | Pass / 17152 / 0.07 |
| Spell, local GCC 13.3 | C++23 O2, cache depth 0 | Pass / 17408 / 0.02 |
| Controls, local GCC 13.3 | C++23 O0 | Pass / 17536 / 0.02 |
| Controls, local GCC 13.3 | C++23 O2, cache depth 0 | Pass / 17536 / 0.02 |
| Controls, local GCC 13.3 | C++23 O0, `-m32 -DADDRESS_BITS=32` | Pass / 17280 / 0.04 |
| Spell, CE GCC 16.2 | C++23 O2, cache depth 0 | Pass, no diagnostics |
| Controls, CE GCC 16.2 | C++23 O2, cache depth 0 | Pass, no diagnostics |
| Spell, CE Clang 22.1.0 | C++23 O2 | Reject: array is too large |

The initial root-shaped prototype also passed GCC 16.2 O2 with default caching.
Clang 18.1.3 rejected that prototype at the same array-type boundary as the final
Clang 22.1.0 run. No portability conclusion depends on the diagnostic fallout
from Clang recovering with a smaller placeholder type.

The 32-bit controls use eight arrays of 2^30 bytes: an eight-GiB logical heap
through four-byte pointers. No standard-library headers or multilib linking
are required for this syntax-only test. This is not a 32-bit runtime execution.

A final GCC 13.3 C++23 O0 object build also passed under the same limits
(0.08 seconds, 17280 KiB peak). Its output object was 944 bytes; `size` reported
32 bytes of text and zero data/BSS. There is no giant runtime storage artifact.
The new reader and handoff links all resolved, and Windows Git's whitespace
check passed. See the papercut log for the initial cross-environment Git mismatch.

The positive [controls](../../tricks/astral-heap/experiments/controls.cpp) cover:
far offsets; reading unmentioned zeroes; overwrites and clearing; same-array
pointer differences; same-offset and different-offset cross-allocation pointer
inequality; and an implicit four-EiB aggregate copy with later independent
mutation. Every array remains live until after these checks.

All five negative controls rejected for the intended reason on local GCC 13.3,
C++23 O0 (each 0.02–0.03 seconds, at most 17664 KiB):

| `PROBE` | Change | Diagnostic reason |
| --- | --- | --- |
| 1 | `new realm` without `{}` | Accessing uninitialized array element |
| 2 | Read `bytes[extent]` | Subscript outside array bounds |
| 3 | Omit deallocation | Allocated storage has not been deallocated |
| 4 | Convert pointer to integer | Pointer-to-arithmetic conversion in a constant expression |
| 5 | Export allocated pointer | Result refers to `operator new` allocation |

These are actual diagnostics, not a generic claim that all compiler failures
validate a negative test. The resource wrapper's exit code alone is not a compiler
verdict: the compiler's own return code and diagnostics were inspected.

## Implementation and prior art

Read the source for the exact local GCC release rather than inferring a sparse
implementation from speed alone:

- `gcc/tree.def`, lines 479–497: `CONSTRUCTOR` has indexed entries, supports
  index ranges, and gives absent entries zero initialization unless its
  no-clearing flag applies.
- `gcc/cp/constexpr.cc`, around lines 2731–2766: intercepted allocation creates
  an artificial declaration, registers its identity, and returns its address.
- `find_array_ctor_elt`, around line 3842: indexed entries support lookup and
  insertion. Sparse lookup uses binary search; do not call this a hash table.
- `cxx_eval_array_reference`, around lines 4244–4302: absent, initialized array
  entries yield value initialization instead of allocating every preceding slot.

Links to these exact release files are in the explainer. Compiler memory still
grows with explicitly represented contents; insertions and copies have real costs.
No asymptotic guarantee for arbitrary aggregate programs is claimed.

P0784R1 (2018) explicitly describes metadata-rich evaluator pointers. Arthur
O'Dwyer's 2023 constexpr string article explains simulated memory and escape
restrictions. Credit both as background. Queries for constexpr plus exabyte,
exbibyte, terabyte, sparse arrays, virtual heaps, and exceeding address space
found no verified exact match for this construction. The symbolic-memory idea
is not new; the particular demonstration was independently derived here. Do not
upgrade a limited prior-art search into a historical-priority claim.

No roughly-Turing claim, new isolated machine, runtime allocation stress test,
or further container project is implied. The spell is deliberately complete at
this boundary; the tabled zero-storage owning vector stays tabled.
