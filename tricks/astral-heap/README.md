# Astral heap

**Eight-byte pointers address a 32-exbibyte heap.**

[The spell](astral-heap.cpp) creates eight independent, simultaneously live
objects. Each contains a four-exbibyte array. It writes different values at both
ends of every array, reads untouched zeroes in their interiors, checks pointer
identity, and deletes everything. GCC evaluates all of this at compile time.

That is `8 × 2^62 = 2^65` bytes: twice the entire address space of a flat
64-bit machine. `sizeof(unsigned char*)` and `sizeof(realm*)` are still eight.
The compiler does not acquire 32 EiB of real RAM. The trick is that its memory
model can represent this heap at all—and cheaply.

## First, ask for one impossible object

```cpp
struct realm { unsigned char bytes[1ull << 62]; };
```

This type contains an ordinary built-in array. There is no sparse container in
our source, and its size really is `2^62` bytes on the tested GCC target.

Inside a constant evaluation, create it with aggregate initialization:

```cpp
auto world = new realm{};
world->bytes[0] = 17;
world->bytes[(1ull << 62) - 1] = 99;
// The other elements are still zero.
delete world;
```

GCC can represent the initial array with an empty initializer and an implicit
zero default. The writes add entries for particular indices. It need not create
or visit four quintillion individual byte values.

The form matters. On GCC 13.3, the seemingly equivalent
`new unsigned char[1ull << 62]{}` instead reaches the constexpr loop limit while
initializing the array. Wrapping the array in an aggregate and writing
`new realm{}` takes the compact path. A local built-in array initialized with
`{}` also worked in the one-EiB probe.

## Then exhaust the address space without exhausting memory

```cpp
realm* worlds[8];
for (auto& world : worlds) world = new realm{};
```

All eight allocations remain alive together. Each is separately addressable;
writes to the same offset in different worlds remain independent. No flat
64-bit runtime address assignment can accommodate all those bytes at once.

GCC's evaluator does not need to assign such addresses. Its pointers retain
the identity of an artificial allocation plus the offset within it. Eight-byte
`sizeof` describes the target's pointer representation; it does not describe the
compiler's internal representation of the pointer value.

This explanation is supported by the implementation. GCC 13.3's allocation
handling creates artificial declarations and returns their symbolic addresses.
Its array evaluator searches explicit initializer entries and supplies the
value-initialized element when an entry is absent.
[Allocation handling](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/gcc/cp/constexpr.cc#L2731),
[array reads](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/gcc/cp/constexpr.cc#L4244),
[initializer representation](https://github.com/gcc-mirror/gcc/blob/releases/gcc-13.3.0/gcc/tree.def#L479).

The loops only arrange and inspect the objects. The phenomenon is the evaluator
supplying a sparse heap with symbolic pointers, beyond the target's flat address
capacity. It is independent of the GCC memoization spell: disabling constexpr
function caching preserves the result. No lifetime probe or mutable compiler
cache is used.

## Evidence and limits

Successful checks used `-Wall -Wextra -pedantic-errors`; local checks also used
`-fsyntax-only`. No constexpr resource limits were raised.

| Configuration | Result |
| --- | --- |
| GCC 13.3.0, C++20, O0 | Spell passes |
| GCC 13.3.0, C++23, O2, `-fconstexpr-cache-depth=0` | Spell and controls pass |
| GCC 13.3.0, C++23, O0 | Controls pass |
| GCC 16.2, C++23, O2, cache disabled | Spell and controls pass |
| GCC 13.3.0, `-m32`, C++23, O0 | Adapted controls: eight live 1-GiB objects, four-byte pointers |
| Clang 18.1.3 and 22.1.0, C++23 | Reject the four-EiB array type as too large |

Local GCC builds ran under a **512-MiB virtual-memory cap** and completed in
0.02–0.07 seconds in the recorded verification run. The largest reported compiler
child-process peak was about **17 MiB**. These are measurements of this small
example, not a performance guarantee for arbitrary giant arrays.

[Controls](experiments/controls.cpp) also check high offsets, pointer differences
within an array, independent identities, overwrites, clearing, and a four-EiB
aggregate copy whose subsequent changes remain independent. Deliberately reading
uninitialized storage, accessing one past the end, leaking an allocation,
converting a pointer to an integer, or exporting an allocated pointer all fail
constant evaluation on the checked GCC 13.3 configuration.

This is a GCC implementation experiment, not a promise that ISO C++ requires
every compiler to accept a heap of this size. It does not create runtime memory,
make pointer-to-integer conversions work, or let the heap survive the constant
evaluation. More touched elements still cost compiler memory and work. The
32-EiB total must not be calculated in a 64-bit `size_t`: that multiplication
would wrap. Each individual object's size fits.

From the repository root:

```sh
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -fsyntax-only tricks/astral-heap/astral-heap.cpp
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -fsyntax-only tricks/astral-heap/experiments/controls.cpp
g++ -std=c++23 -O0 -m32 -DADDRESS_BITS=32 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/astral-heap/experiments/controls.cpp
```

Compile the controls with `-DPROBE=1`, `2`, `3`, `4`, or `5` for the intentional
rejections listed above, respectively. Do not turn the giant example into a
runtime allocation test.

## Provenance

Independently assembled and tested by Tor Shepherd and OpenAI's Codex in the
Constexprnomicon research session of September 9, 2026. No historical-first claim.

Symbolic constexpr memory is established implementation practice.
[P0784R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0784r1.html)
already explains why evaluator pointers need metadata beyond a runtime address.
Arthur O'Dwyer's 2023
[constexpr memory explanation](https://quuxplusone.github.io/blog/2023/09/08/constexpr-string-firewall/)
describes the simulated stack and heap and why their pointers cannot escape.
Those are background for the mechanism, not claimed descriptions of this
32-EiB construction. Targeted searches found no exact matching demonstration;
that limited search cannot establish novelty.

This is a standalone storage phenomenon. No isolated-feature or roughly-Turing
claim is made. [Research notes](../../.agents/notes/17-astral-heap.md).
