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

## September 11 follow-up: recoverable pointer payload

Tor asked verbatim:

> Can you read up on “astral heap”? Very weird finding in this report that you can have a 65 bit address space at compile time even though pointers are 8 bytes at runtime. Does this give us a mechanism for storing more information than it appears, similar to the “zero size integer” trick? My assumption is that this is not workable due to the constraint of “no pointer punning at compile time”

**Result:** yes, with a shared codebook and GCC's constantness probe. The
[retained source](../../tricks/astral-heap/experiments/pointer-payload.cpp) maps
`{bool high, uint64-like low}` to one byte pointer using three bank bits and
62 offset bits. Recover the bank with
`__builtin_constant_p(p - worlds[bank]->bytes)`, then perform the matching
same-array subtraction. Pointer-to-integer conversion is unnecessary.

This is an application of Astral heap plus the already-used constantness-probe
technique, not a newly discovered compiler builtin or a historical-first claim.
The codebook is eight fixed, simultaneously live, separately allocated arrays.
All are zero-initialized and no payload byte is written. It is shared among
encoded values and does not change when values are encoded, copied, reassigned,
or swapped. A pointer can therefore select any of 2^65 positions against the
same context. The construction is more specific than merely storing an integer
in a heap object and pointing to it.

### Bounded derivation and verification

- First probe allocated two realms, chose `b->bytes + extent - 123`, and checked
  that subtraction from `a->bytes` was not constant to the builtin, subtraction
  from `b->bytes` was, and the latter recovered the offset. GCC 13.3 accepted at
  C++23/O0 with constexpr caching disabled.
- The retained source checks 8 banks times 64 offset patterns (each single bit,
  zero, maximum), decoding pointer copies after reassigning the original. It also
  checks two independent values with identical low 64 bits and opposite high
  bits, swaps them, and verifies unchanged zero pointees.
- Final positive runs: GCC `13.3.0`, C++20/O0 and C++23/O2 with
  `-fconstexpr-cache-depth=0`. Both code 0, no diagnostics, around 0.025 seconds.
  All runs use `-Wall -Wextra -pedantic-errors -fsyntax-only`, a 512-MiB address
  space cap, 15-second CPU cap, and 20-second parent timeout. No compiler limits
  were increased.
- Final negative runs: GCC 13.3, C++23/O0, same warnings and resource caps.
  `BAD_SUBTRACTION` rejects the evaluated subtraction between different
  allocations; `BAD_PUN` rejects pointer-to-arithmetic conversion;
  `UNKNOWN_POINTER` reaches the decoder's throw for a ninth, unrelated allocation.
  Each returns compiler code 1 with the corresponding diagnostic, around
  0.024–0.029 seconds.
- Initial negative controls merely cast the forbidden expression to `void`.
  GCC accepted both discarded computations. This was insufficient to force the
  operation; changing each to contribute to the final `okay` result made both
  reject as intended. Keep this correction, not the initial expected verdict.
- Compiler Explorer discovery returned `g162`, `g133`, `g122`, and `clang2210`.
  The request to compile the new source with `g162` was blocked by automatic
  approval review as disclosure of locally sourced repository code to an
  untrusted service. No remote compiler verdict exists, and the blocked upload
  was not retried via another route. Local evidence suffices for this finding.

### Interpretation and boundaries

The reader-facing walkthrough is in the parent README. Keep these distinctions:

1. The value's representation is an eight-byte target pointer, but its decoder
   needs the shared 64-byte base-pointer table and the eight logical arrays.
   It is not a self-contained eight-byte total-storage replacement for `uint65`.
2. The extra information is allocation identity plus offset. `sizeof` describes
   target layout, not the evaluator's internal symbolic value representation.
   P0784R1 supplies historical background for metadata-rich constexpr pointers.
3. Ordinary pointer-value copies retain identity and offset, unlike copying the
   Empty bits wrapper whose lifetime state is associated with its subobjects.
4. The builtin is a GCC extension, and its documented zero return is not a
   general proof of invalidity. The bank test is empirically established for
   these live allocations and pointers. Decode only encoded interior pointers;
   there is no general arbitrary-pointer validator or one-past encoding here.
5. Integer-to-chosen-symbolic-pointer encoding does not solve arbitrary-pointer
   bit extraction, does not revive the empty owning vector, and cannot export
   the allocated codebook or its pointer values to runtime. A decoded ordinary
   scalar/aggregate result can leave constant evaluation.
6. Eight banks establish 65 recoverable bits; no universal 65-bit evaluator cap
   follows. More banks would add choices with more setup and linear decoding
   work. No expanded-bank construction was tested in this session.

Read current GCC builtin documentation and draft `expr.add` for the exact
non-evaluation and subtraction rules; links are in the explainer. The research
did not attempt a builtin-free bank decoder, runtime giant allocations, or a
new isolated computational machine. No pending follow-up is implied.
