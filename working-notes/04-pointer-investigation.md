# Pointer investigation — tabled

Tor's proposal was to abstract the empty integer into `empty<T>`, then store the
one-pointer vector's `slot*` in `empty<slot*>`. That would make the owning vector
an empty class, potentially costing no additional layout storage as a base.
**No complete construction was found.** Tor explicitly allowed shelving it.

The distinction throughout this search is between preserving pointer identity,
recovering a typed pointer from `void*`, and serializing a pointer into ordinary
integer bits that the empty bit store can encode. Several probes achieved the
first two; none achieved the third.

## The scalar bridge works

`empty-values-probe.cpp` derives `empty<T>`
from the Clang empty integer. `set` bit-casts `T` to an unsigned-character array,
packs it into an `unsigned long long`, and writes those logical bits. `get`
unpacks the bytes and bit-casts back.

Clang 22.1.0 passed `int` and `double` checks at C++23 `-O0`/`-O2`, including
negative values, two independent double objects, mutation, `sizeof == 1`, and
`is_empty`. The experiment is limited to suitable bit-castable types fitting in
the 64-bit store. Padding, indeterminate representations, and other bit-cast
restrictions were not solved; its packing assumes eight-bit bytes on the tested
target. It is not an unrestricted `empty<T>` implementation.

With `-DPOINTER`, the same file allocates an `int`, attempts to save its pointer,
then compares and dereferences the recovered pointer before deleting it. Clang
rejects the pointer bit-cast. The request expanded the root empty-bits
implementation before compilation. The wrapper itself was:

```cpp
// Place after the root empty-bits.cpp implementation.
#include <bit>
#include <array>

template<class T> struct empty : integer {
    static_assert(sizeof(T) <= sizeof(unsigned long long));
    constexpr empty(T value = {}) { set(value); }
    constexpr void set(T value) {
        auto bytes = std::bit_cast<std::array<unsigned char, sizeof(T)>>(value);
        unsigned long long n = 0;
        for (unsigned i = 0; i < sizeof(T); ++i)
            n |= 1ull * bytes[i] << (i * 8);
        integer::set(n);
    }
    constexpr T get() const {
        std::array<unsigned char, sizeof(T)> bytes{};
        auto n = integer::get();
        for (unsigned i = 0; i < sizeof(T); ++i) bytes[i] = n >> (i * 8);
        return std::bit_cast<T>(bytes);
    }
};
```

## First pointer attempts

`pointer-probe.cpp` selects six routes with
`-DROUTE=N`: `bit_cast`, `reinterpret_cast`, copying into an integer, inactive
union punning, copying into a byte array, and subtraction from null. Both
compilers rejected the tested attempts except GCC's direct integer round trip.

`pointer-roundtrip.cpp` isolates that result:
GCC accepts a pointer → `uintptr_t` → pointer round trip, including identity and
dereference checks, at C++23 `-O0`/`-O2`; Clang rejects it. Enabling `READ_BIT`
and asking GCC for even the low bit fails. The decisive shape is:

```cpp
#include <cstdint>
static_assert([] {
    auto p = new int(42);
    auto n = reinterpret_cast<std::uintptr_t>(p);
    // Reading n & 1 here breaks the GCC result.
    auto q = reinterpret_cast<int*>(n);
    bool ok = q == p && *q == 42;
    delete p;
    return ok;
}());
```

The six GCC-only followups in
`pointer-integer-probe.cpp`, C++23 `-O2`:

| Route | Attempt | Result |
| --- | --- | --- |
| 1 | Separate `encode` and `decode` functions | Rejected |
| 2 | Add `sizeof(int)` to the apparent integer | Rejected |
| 3 | Reconstruct it one bit at a time | Rejected |
| 4 | Bit-cast the apparent integer through bytes | Rejected |
| 5 | Store it in a `uintptr_t` array, then cast back | Accepted |
| 6 | Read `n & 1` before casting back | Rejected |

The useful interpretation is that some expressions can preserve a symbolic
pointer or fold the round trip without materializing its address. That is an
inference from the controls, not a complete diagnosis of GCC's internal value
representation. An object identity and offset tracked by an evaluator are not
automatically a numerical machine address.

The empty store works on Clang's tested evaluator; GCC is the compiler accepting
the simple integer round trip. Neither observation bridges the missing readable
bits, and they cannot simply be combined across compilers.

## Broader sweep

The saved sweep has **36 distinct programs and 82 retained configurations**.
The baseline is C++23 `-O2` on GCC 16.2 and Clang 22.1.0. Selected cases also
have `-O0` and/or Clang's experimental new interpreter flag. These counts come
from the scratch result records inspected during this handoff. The table below
records the tested families and their limitations; the raw records are not
committed with these working documents.

| Family / saved case names | Observed outcome |
| --- | --- |
| `c_cast_bits`, `reference_pun`, `may_alias` | No readable pointer integer; both reject the tested sources |
| `bitcast_wrapper`, `bitcast_nested` | Wrapping pointers in aggregates/arrays does not bypass bit-cast restrictions |
| `void_roundtrip`, `launder_roundtrip` | GCC accepts same-type recovery; Clang rejects the C++23 cast |
| `void_byte_read`, `assume_aligned` | Neither source yields readable pointer representation |
| `memmove_integer`, `memcpy_inline`, `memcmp_representation` | No successful representation extraction; some builtins unavailable on GCC |
| `memcpy_same_type_control` | Clang accepts copying a pointer into another pointer; GCC rejects this constexpr builtin use |
| `memchr_representation` | Inconclusive as a memory-search technique: the particular builtin was unavailable or called with the wrong argument type |
| `union_common_sequence`, `lifetime_reuse` | Inactive common-sequence read / replacement without initialized numeric bits rejected |
| `pointer_difference`, `unrelated_difference` | Subtraction from null or unrelated allocation rejected |
| `unrelated_ordering` | GCC accepts `p < q || q < p`; Clang rejects |
| `pointer_order_value` | Actually materializing the ordering value is rejected by both |
| `alignment_low` | Clang knows alignment 4 for these `int` pointers; GCC lacks the builtin |
| `alignment_high`, `align_down`, `alignment_probe_bits` | No additional address bits recovered; high alignment cannot be established |
| `constant_probe_cast_bits` | The assertion expecting recognized numeric bits fails on both |
| `constant_probe_identity` | Both recognize the identity expression, even though Clang rejects executing the direct round trip |
| `container_of` | The tested byte-offset recovery from a member pointer is rejected by both |
| `allocation_cache` | Two equal-argument allocation calls do not yield the same pointer in the tested assertion |
| `global_state_probe` | No usable hidden pointer registry: reading the global stash in the final constant evaluation fails |
| `source_location_*`, `allocator_*` | Same-type erasure loopholes described below; wrong-type/bit extraction attempts fail |

Several negative results need careful wording:

- `memchr_representation` specifically called `__builtin_char_memchr(&p, ...)`.
  GCC did not recognize that builtin; Clang expected `const char*` and rejected
  the argument type. This does **not** semantically exhaust `__builtin_memchr`
  or all memory-search formulations.
- Clang's known natural alignment is information supplied by the pointee type;
  it does not reveal the pointer's identity. Asking about 4096 alignment failed.
- GCC's accepted ordering disjunction can be folded without exposing an order.
  The followup that needed the actual boolean failed. No allocator ordering
  primitive was found.
- A builtin recognizing the round-trip equality is not proof that the casts
  are independently executable in a core constant expression. Recognition and
  folding can be less strict than the operation being probed.
- The global-stash attempt failed as a usable registry. That does not by itself
  prove that no intermediate mutation occurred; see the separate mutation
  observations in [compiler state](03-compiler-state.md#side-effects-are-not-a-transaction).
- Exit 1 sometimes means an unsupported option/builtin or an assertion that the
  desired property is false, not a diagnosed prohibition of an entire approach.

## The library-name loophole

This heap example passed both default evaluators at C++23 `-O0`/`-O2`:

```cpp
namespace std { struct source_location {
    template<class T> static constexpr T* current(void* p) {
        return static_cast<T*>(p);
    }
}; }

static_assert([] {
    auto p = new int(42);
    void* erased = p;
    auto q = std::source_location::current<int>(erased);
    bool ok = q == p && *q == 42;
    delete p;
    return ok;
}());
```

The source is stored as `source_location_heap`. The related local-object case
is `source_location_cast`. Moving the identical cast body into
`ordinary::source_location::current` made the control fail on GCC `-O2` and
Clang `-O0`/`-O2`. The spelling is doing work.

Both compiler implementations contain standard-library accommodations based on
the enclosing declaration. A fake `std::allocator<T>::allocate(void*)` also
received the same-type `void*` cast allowance in its tested local-object source
(GCC `-O2`, Clang `-O0`/`-O2`). These are deliberately nonconforming declarations
inside `std`, not supported customizations. They should not be combined with
the real library declarations or presented as ordinary `source_location` usage.

Wrong-pointee-type casts and attempts to read pointer storage as an integer
still failed. The loophole recovers the actual pointee type through erasure;
it does not turn an `int*` object into its byte representation.

### Interpreter controls and source inspection

Four Clang cases were also run with
`-std=c++23 -O2 -fexperimental-new-constant-interpreter`:

| Case | Default Clang | New interpreter |
| --- | --- | --- |
| `source_location_cast` | Accepts | Rejects |
| `allocator_cast` | Accepts | Accepts |
| `void_roundtrip` | Rejects | Rejects |
| `bitcast_wrapper` | Rejects | Rejects |

Do not cite the new interpreter documentation as proof that the default runs
used it. The session inspected the pinned
[Clang 22.1.0 AST evaluator source](https://github.com/llvm/llvm-project/blob/llvmorg-22.1.0/clang/lib/AST/ExprConstant.cpp):
`IsDeclSourceLocationCurrent`, the void-pointer cast gate, pointer-to-integral
handling, and memory builtins. The cast accommodation still checks similarity
of the actual pointee type. The comments explain a source-location compatibility
workaround for older libstdc++. Clang's
[memory builtin documentation](https://clang.llvm.org/docs/LanguageExtensions.html#memory-builtins)
also explains why a same-type pointer copy is different from copying its
representation into integer storage.

The session also read GCC's
[current `gcc/cp/constexpr.cc`](https://github.com/gcc-mirror/gcc/blob/master/gcc/cp/constexpr.cc),
including `is_std_source_location_current` and the cast exceptions for standard
library functions. That source was **master**, not a pinned GCC 16.2 source
snapshot. It supports the direction of the explanation; the actual reported
acceptance is from the saved 16.2 compiler requests.

Prior-art search leads included
[a 2017 Clang discussion of constexpr `void*` casts and container-of](https://lists.llvm.org/pipermail/cfe-dev/2017-August/055076.html)
and [GCC PR95307](https://gcc.gnu.org/PR95307). Those are context, not evidence
that our modern pointer-bit extraction worked. An accepted function definition
in an old example is weaker than forcing the cast to execute. Not every linked
issue page was retrievable, and no new issue was filed.

## Parking point

There is a small scalar `empty<T>` demonstration and a separate, striking
type-erasure loophole. There is no working pointer serializer, no proof that
one is impossible, and no empty owning vector to check in as a supported spell.
If Tor returns to this, start from these exact controls. A new direction should
identify the missing bridge explicitly: recoverable allocation identity stored
without a pointer member, or a way to recover ordinary pointer bits that the
empty store can carry. Merely accepting another cast is insufficient.
