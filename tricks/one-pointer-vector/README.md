# One-pointer vector

**One pointer; the compiler remembers the rest.** Union lifetimes identify
occupied slots, spare capacity, and—in the C++26 version—the final slot.

| Version | Source | How capacity ends |
| --- | --- | --- |
| C++23 builtin | [one-pointer-vector.cpp](one-pointer-vector.cpp) · [Godbolt sketch](https://godbolt.org/z/MscKzxdjf) | Failed constantness probe beyond the allocation |
| C++26 library | [one-pointer-vector-cxx26.cpp](one-pointer-vector-cxx26.cpp) · [Godbolt, second editor](https://godbolt.org/z/on8WTjYKq) | Active `end` member in the reserved final slot |

Both handles store only `slot* p`, initially null. Growing from size `n`
allocates `2 * n + 2` slots, of which `2 * n + 1` are usable. Capacities grow
through 1, 3, 7, ... . Each allocation reserves one final slot; no value of `T`
is reserved. Assigning a union with active `value` pushes an element;
activating `empty` pops it.

## The builtin version: probe the empty marker

```cpp
union slot { char empty = 0; T value; };
slot* p = nullptr;
```

The occupied prefix has inactive `empty` members; every following slot has
active `empty`. Probing that scalar marker works for the tested structs and
pointers too, without asking whether an arbitrary `T` is recognized as constant.

```cpp
consteval int size() const {
    int n = 0;
    while (p && !__builtin_constant_p(p[n].empty)) ++n;
    return n;
}
consteval int capacity() const {
    int n = size();
    while (__builtin_constant_p(p[n].empty)) ++n;
    return n - !!p;
}
```

`size()` stops at the first empty marker. `capacity()` continues through the
empty tail until the allocation ends, then subtracts the reserved slot.

| Slot state | Probe of `empty` |
| --- | --- |
| Occupied (`value` active) | 0 |
| Spare or final (`empty` active) | 1 |
| Outside the allocation | 0 on the tested compilers |

The builtin lets failed evaluation become zero. That is observed compiler
behavior, not a general pointer-validity API: GCC documents zero as failure to
establish constantness, not proof of invalidity.
[Builtin documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html).

## The C++26 version: give the final slot its own member

`std::is_within_lifetime` asks about an object's lifetime directly. Unlike the
builtin probe, it **rejects a one-past pointer instead of returning false**.
Its [specification](https://eel.is/c++draft/meta.const.eval) requires a pointer
to an appropriate object; both checked implementations reject the
[boundary control](experiments/within-lifetime-controls.cpp) with `ONE_PAST`.

The owner can avoid that query by distinguishing the already-reserved last slot:

```cpp
union slot { char empty = 0; T value; char end; };
// When growing from size n:
auto q = new slot[2 * n + 2];
q[2 * n + 1] = {.end = 0};
```

Every query now concerns a subobject of an allocated slot:

```cpp
consteval int size() const {
    int n = 0;
    while (p && std::is_within_lifetime(&p[n].value)) ++n;
    return n;
}
consteval int capacity() const {
    int n = size();
    while (p && !std::is_within_lifetime(&p[n].end)) ++n;
    return n;
}
```

Occupied slots have active `value`; unused slots have active `empty`; the final
slot has active `end`. No out-of-bounds probe occurs. The third alternative
adds no byte to the tested `int` slot, no field to the handle, and no allocation
slot beyond the original sentinel. The lifetime query also avoids the builtin's
limitations on recognizing aggregate values as constant.

## What about the original pointer-only span?

The starting trick scanned a plain array through an ordinary pointer:

```cpp
constexpr int sum(const int* p) {
    int total = 0;
    while (__builtin_constant_p(*p)) total += *p++;
    return total;
}
```

In the tested required constant evaluations, `{10,20,0,40}` sums to 70 and the
first one-past read makes the probe false. Zero is ordinary data. See the
[original span](https://godbolt.org/z/474oesPvP) and
[retained controls](experiments/constantness-controls.cpp).

`std::is_constant_evaluated()` only reports the evaluation context; it remains
true at the array's end. `if consteval` makes the same context distinction.
`requires { *p; }` checks an unevaluated expression, so it does not discover
bounds either. All three controls fail on the one-past read. `if constexpr`
requires a valid constant condition: `if constexpr (*p == 10)` rejects the
ordinary function parameter. In particular,
`if constexpr (std::is_constant_evaluated())` always selects its true branch,
even for runtime calls. See [the language rule](https://eel.is/c++draft/stmt.if).

A stronger requirement works when the pointer and index are template arguments:

```cpp
template<const int* P, unsigned I = 0> consteval unsigned count() {
    if constexpr (requires { typename std::integral_constant<int, P[I]>; })
        return 1 + count<P, I + 1>();
    else return 0;
}
```

The [complete template example](experiments/template-span.cpp) counts all four
values. An out-of-bounds read cannot form the constant template argument, so
substitution makes the [type requirement](https://eel.is/c++draft/expr.prim.req.type)
false. GCC 16.2 and Clang 22.1.0 both pass it in C++23/O2.

This changes the interface: it works for suitable constant objects supplied as
template arguments, not arbitrary changing local state or allocations passed
through an ordinary pointer. An ordinary parameter does not become a template
argument merely because its function is `consteval`. The plain span cannot
retrofit the owning vector's union sentinel onto an existing array. Neither
scan discovers an arbitrary shorter subspan's intended endpoint.

## Limits and compiler evidence

Both vectors expect trivially copyable/destructible element types suitable for
the union operations. Keep the owning handle uncopied, pop only when nonempty,
and index live elements. Queries scan linearly; allocation and deletion occur
within the same constant evaluation. Slots form a `slot[]`, not a contiguous
`T[]`. These remain small container sketches, not general `std::vector` replacements.

| Implementation | Verified outcome |
| --- | --- |
| Builtin, GCC 16.2 / Clang 22.1.0, C++23, O0/O2 | Full checks pass |
| Library, Clang 22.1.0/libc++, C++26, O2 | Full checks pass |
| Library, GCC trunk 20260912, C++26, O2 | Full checks pass |

The full checks cover integers, doubles, booleans, aggregates, pointers including
null, and a class with a converting constructor. Both versions check that the
handle occupies one pointer and an `int` slot occupies `sizeof(int)`.
The builtin Godbolt sketch uses C++20 and shorter checks. GCC 13.3 rejects the
unchanged full builtin source; see the
[cleanup check](../../.agents/notes/16-repository-cleanup.md#verification).
GCC 16.2 and the tested Clang default library setup lack the new library API;
select libc++ for the C++26 Clang version.

## Try both

Run from the repository root with the matching compiler installed:

```sh
g++-16 -std=c++23 -O2 -c tricks/one-pointer-vector/one-pointer-vector.cpp -o /tmp/one-pointer-vector.o
clang++-22 -std=c++2c -stdlib=libc++ -O2 -c tricks/one-pointer-vector/one-pointer-vector-cxx26.cpp -o /tmp/one-pointer-vector-cxx26.o
```

[Research evidence and negative controls](../../.agents/notes/22-constantness-alternatives.md) ·
[Shared provenance](../../docs/PROVENANCE.md)
