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

## Follow-up: a 65-bit value in one eight-byte pointer

Yes, the extra symbolic address information can carry a recoverable payload.
[pointer-payload.cpp](experiments/pointer-payload.cpp) uses the eight arrays as
a shared, immutable codebook. An encoded value is just an `unsigned char*`:

```cpp
// bank has 3 bits; offset has 62 bits.
auto p = worlds[bank]->bytes + offset;
```

That selects one of `8 * 2^62 = 2^65` interior byte positions. No array element
is written to encode a value. Many independent pointer values can share the
same eight arrays; copying or swapping those pointers preserves the payload.

The missing operation was recovering the bank without converting the pointer
to an integer. On the tested GCC, this probe provides it:

```cpp
for (unsigned bank = 0; bank < 8; ++bank)
    if (__builtin_constant_p(p - worlds[bank]->bytes)) {
        auto offset = p - worlds[bank]->bytes;
        // bank and offset recover the original 65 bits.
    }
```

Subtracting pointers within the same array gives their index difference;
subtracting pointers into different arrays is undefined.
[Pointer subtraction](https://eel.is/c++draft/expr.add).
Here GCC returns zero from the builtin for a different array, and one for the
matching array. Only the matching subtraction is then evaluated normally.
GCC documents that the builtin's operand is not evaluated, and that zero means
it could not establish a constant, not that it proved an expression invalid.
[Builtin documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html).
The particular bank-discrimination behavior is compiler evidence, not a general
pointer-validity API or an ISO guarantee.

The source accepts an ordinary `{bool high, unsigned long long low}` input and
recovers both fields from the pointer. The extra bit is not a template argument
or a second member alongside the encoded pointer. Its decoder is for interior
pointers returned by its encoder, while the shared codebook remains alive.

The costs and boundaries matter:

- Each encoded value is one eight-byte pointer. The shared context additionally
  contains eight base pointers (64 target bytes) and eight huge logical arrays,
  represented sparsely by GCC. This is not eight bytes of total compiler memory.
- Unlike Empty bits' lifetime state, ordinary pointer value copies preserve this
  encoding. It still uses evaluator metadata rather than extra runtime bits.
- This encodes a chosen integer as a symbolic pointer. It does not serialize
  an arbitrary pointer's representation, permit pointer punning, or solve the
  tabled empty owning vector problem.
- The encoded pointer and its allocated codebook stay inside one constant
  evaluation. Decode to ordinary values before releasing the arrays. There is
  no 65-bit runtime pointer or runtime compression claim.
- Eight arrays give 65 bits; the original demonstration does not establish a
  65-bit limit on GCC's evaluator. More banks would add choices and setup/search
  cost. Larger-bank variants have not been tested here.

The follow-up passed **local GCC 13.3.0**, C++20/O0 and C++23/O2 with
`-fconstexpr-cache-depth=0`, using warnings and pedantic errors. Assertions check
512 round trips spanning every bank, every offset bit, zero and maximum offsets,
copying, reassignment, swapping, and two simultaneous values differing only in
bit 64. Each sampled pointee remains zero. Direct cross-allocation subtraction,
pointer-to-integer conversion, and decoding an unrelated allocation are separate
intentional rejection controls. After Tor explicitly authorized public Godbolt
uploads, this source also passed GCC 16.2 at C++23/O2 with caching disabled.
There is no Clang result for this follow-up; the compiler table below concerns
the original heap spell.

```sh
g++ -std=c++20 -O0 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/astral-heap/experiments/pointer-payload.cpp
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -fsyntax-only tricks/astral-heap/experiments/pointer-payload.cpp
```

Add `-DBAD_SUBTRACTION`, `-DBAD_PUN`, or `-DUNKNOWN_POINTER` separately to the
C++20/O0 command to reproduce the expected rejections.

## Removing the builtin: label the banks

The constantness probe is **not necessary** if we give the fixed codebook
ordinary bank labels. The complete
[tagged-pointer-payload.cpp](experiments/tagged-pointer-payload.cpp) uses no
compiler builtins, pointer-to-integer conversion, or reinterpretation.

First, make each slot carry its bank's fixed label:

```cpp
struct cell { const unsigned char bank; };

template<unsigned Bank>
struct slot : cell {
    constexpr slot() : cell{Bank} {}
};
```

Each of the eight banks contains an array of 2^62 of its own slot type:
`slot<0>`, `slot<1>`, and so on. A slot still occupies one byte on the checked
target. Bank zero is all zeroes; bank seven is all sevens. GCC handles these
uniform initializations cheaply, including the nonzero labels. No filling loop
over the array appears in our source, and the tests raise no constexpr limits.

An encoded value has the common type `cell const*`. Encoding still just chooses
a bank and an offset; it never modifies the arrays. To decode:

1. Read `p->bank`: this supplies the three bank bits with an ordinary member read.
2. Dispatch to the corresponding bank type. Within the branch for bank `B`, use
   `static_cast<slot<B> const*>(p)` to recover the pointer to its array element.
3. Subtract that bank's array base to recover the 62-bit offset.

The cast matters. `p` points to a **base subobject** of a slot; subtracting such
base pointers as if they formed a `cell[]` would be wrong. The selected downcast
returns the actual `slot<B>` array-element pointer, so the subsequent subtraction
is within its real array. These are ordinary
[base-to-derived casts](https://eel.is/c++draft/expr.static.cast) and
[same-array subtraction](https://eel.is/c++draft/expr.add).

The source uses a small fold expression to dispatch among the eight bank types.
Those template parameters define the fixed bank vocabulary, not the changing
65-bit payload. Inputs are ordinary function arguments, and independently
encoded pointers of the same type can hold different values.

This preserves the useful property of the first construction: **one pointer
selects among 2^65 positions in one unchanged, shared codebook**. The supporting
arrays now contain fixed labels rather than all zeroes. This is not allocating
a separate integer object for each value: repeated encoding, reassignment,
copying, and swapping do not change any label or allocate another bank.

The source passes local GCC 13.3 (C++20/O0 and C++23/O2, cache depth zero) and
Compiler Explorer GCC 16.2 (C++23/O2, cache depth zero), all with
`-Wall -Wextra -pedantic-errors`. The same 512 bank/offset round trips pass,
along with copies, swaps, opposite high bits, and untouched far label reads.
Local runs complete in about 0.05 seconds under a 512-MiB virtual-memory cap.

[tagged-controls.cpp](experiments/tagged-controls.cpp) also returns a decoded
`{true, 0xfedcba9876543210ull}` beyond the evaluation after the arrays have been
freed. Its local GCC controls reject a wrong-bank downcast and decoding against
a different codebook's allocation. The decoder's precondition is a pointer
encoded by the same live codebook, not an arbitrary pointer. Encoded pointers
remain transient, and the shared supporting storage still counts.

```sh
g++ -std=c++20 -O0 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/astral-heap/experiments/tagged-pointer-payload.cpp
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -fsyntax-only tricks/astral-heap/experiments/tagged-pointer-payload.cpp
g++ -std=c++23 -O0 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/astral-heap/experiments/tagged-controls.cpp
```

Add `-DWRONG_BANK_CAST` or `-DWRONG_CODEBOOK` separately to the final command
for intentional rejections. Keep the pointer spellings from the source:
`cells + offset` for encoding and decayed `cells` for subtraction. The research
notes record GCC rejecting some equivalent mixed spellings for these giant
arrays. Ordinary C++ operations do not guarantee portable support for a 32-EiB
logical heap; the giant sparse initialization remains implementation-dependent.

For the **unchanged, all-zero** codebook, an equality-only decoder is possible
in principle: compare against all eight bases, decrement the pointer if none
matches, and repeat. All operations stay within the pointed-to array. A small
offset probe passes, but a worst-case 62-bit offset requires roughly 2^62 rounds.
The tagged version supplies a practical decoder without that scan. Direct `<`,
`std::less`, and `std::compare_three_way` did not supply an alternative ordering
of independent allocations in the tested GCC constant evaluation.

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
