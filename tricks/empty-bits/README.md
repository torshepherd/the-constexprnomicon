# Empty bits

**An empty class carries 64 writable logical bits.** Two implementations use the
same storage: the lifetimes of 64 distinct empty base subobjects.

| Version | Source | Lifetime query |
| --- | --- | --- |
| C++23 builtin | [empty-bits.cpp](empty-bits.cpp) · [Godbolt](https://godbolt.org/z/cxqejscT1) | `__builtin_constant_p(p->alive())` |
| C++26 library | [empty-bits-cxx26.cpp](empty-bits-cxx26.cpp) · [Godbolt, first editor](https://godbolt.org/z/on8WTjYKq) | `std::is_within_lifetime(p)` |

## Store the bits in lifetimes

The class inherits `bit<0>` through `bit<63>`. Each base is empty. Destroying
base `I` clears bit `I`; constructing it sets that bit. Both versions satisfy
`sizeof(integer) == 1` and `std::is_empty_v<integer>` on the tested Clang target.
An empty complete object still occupies a byte; the payload has no data members.

The stored number arrives through ordinary function arguments during constant
evaluation. The template arguments identify bit positions, not the number.
Two objects of the same type can hold different values, and repeated calls to
`set(n)` change those values.

## Read them with the builtin

The C++23 version gives each base a non-static member:

```cpp
template<unsigned I> struct bit {
    constexpr bool alive() const { return true; }
};
```

Clang's evaluator accepts that call on a live base and rejects it on a dead one.
The builtin turns that difference into a readable bit. Within the setter:

```cpp
auto p = static_cast<bit<J>*>(this);
if (__builtin_constant_p(p->alive())) std::destroy_at(p);
if (n >> J & 1) std::construct_at(p);
```

The getter probes every base, shifts its result by the bit position, and ORs
those results together. The returned `true` is not stored in the object:
**whether the call can be evaluated** supplies the changing value.

## Read them with the C++26 library

The alternate implementation makes `bit<I>` completely empty, including its
member-function list. It asks about the lifetime directly:

```cpp
auto p = static_cast<bit<J>*>(this);
if (std::is_within_lifetime(p)) std::destroy_at(p);
if (n >> J & 1) std::construct_at(p);
```

The getter likewise uses `std::is_within_lifetime` in its fold. Because this
query is `consteval`, the alternate constructor, setter, getter, and test
lambdas use immediate evaluation. The destructor remains `constexpr`.
The query is specified by [C++26](https://eel.is/c++draft/meta.const.eval);
[P2641R4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2641r4.html)
explains why exposing lifetime information is useful.

`std::is_constant_evaluated()` cannot replace either query: it reports the
current evaluation context, which is the same for all 64 bases. `if constexpr`
requires a valid constant condition; it does not turn a dead-base call into
false. The C++26 library function supplies the actual lifetime observation.

## Limits and compiler evidence

Both destructors revive the bases before their implicit destruction. The state
belongs to evaluator lifetime bookkeeping, so copying the wrapper's bytes does
not copy it. Keep the wrapper uncopied and confined to constant evaluation.

**The standard query does not establish portability of the base replacement.**
The construction still depends on Clang's handling of empty, potentially
overlapping base subobjects. It is compiler behavior as art, not runtime
integer compression.

| Implementation | Verified outcome |
| --- | --- |
| Builtin, Clang 22.1.0, C++23, O0/O2 | All checks pass |
| Builtin, GCC 16.2, C++23 | Rejected |
| Library, Clang 22.1.0/libc++, C++26, O0/O2 | All checks pass |
| Library, GCC trunk 20260912, C++26, O2 | API available; value checks fail |

Both Clang versions of the spell check two independent objects, sixteen
patterns, repeated zero/all-one writes, mutation, size/emptiness, and destruction.
The tested Clang default library setup lacked the new API; select libc++.
GCC 16.2 also lacks the API. These availability failures are separate from
GCC trunk's failed value checks.

## Try both

Run from the repository root; executable names depend on your installation.

```sh
clang++-22 -std=c++23 -O2 -c tricks/empty-bits/empty-bits.cpp -o /tmp/empty-bits.o
clang++-22 -std=c++2c -stdlib=libc++ -O2 -c tricks/empty-bits/empty-bits-cxx26.cpp -o /tmp/empty-bits-cxx26.o
```

[Research evidence](../../.agents/notes/22-constantness-alternatives.md) ·
[Shared provenance](../../docs/PROVENANCE.md)
