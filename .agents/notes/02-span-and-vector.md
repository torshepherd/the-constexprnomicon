# Span, unions, and vector

Compiler names below mean x86-64 GCC 16.2 (`g162`) and Clang 22.1.0
(`clang2210`). The initial experiments used C++20; the root spells were separately
checked in C++23. Optimization and forcing constant evaluation matter.

## The initial pointer-only probe

The essence of Tor's starting idea is:

```cpp
constexpr int sum(const int* p) {
    int total = 0;
    while (__builtin_constant_p(*p)) total += *p++;
    return total;
}
```

In a forced constant evaluation, the tested integer elements yield a recognized
constant and a one-past-end load does not. A zero element is ordinary data.
The fuller example wraps a single pointer in a range with a one-pointer iterator
and empty end sentinel; its operations are `consteval`.

| Forced experiment | GCC | Clang |
| --- | ---: | ---: |
| Sum `{10, 20, 0, 40}` | 70 | 70 |
| Start at `a + 2` | 40 | 40 |
| Start one-past-end or at null | 0 | 0 |
| Mutate local `{3,4,5}` to `{3,40,5}`, then sum | 48 | 48 |
| Allocate `{2,4,8}`, sum, and free | 14 | 14 |
| Sum temporary-evaluation `std::vector{7,0,9}` | 16 | 16 |
| Count vector with capacity 10 and three initialized elements | 3 | 3 |
| Count `"abc"`, including its null terminator | 4 | 4 |
| Probe a live mutable global | 0 | 0 |
| Guard `12 / 0` | Fallback −1 | Fallback −1 |
| Count from first row of `int[2][3]` | 6 | 3 |
| Count two `record { int x; }` elements | 2 | 0 |

The array-boundary discrepancy was isolated without library headers. It is not
evidence that crossing a row boundary is valid ISO C++. A live mutable global
returning false shows why “dereferenceability” is only an intuition. A pointer
also cannot encode an arbitrary intended shorter subspan inside a larger array.

In the main saved example, an explicitly forced call produced 70 while ordinary
calls produced 0 at both optimization levels. A separate smaller probe allowed
GCC `-O2` to fold an ordinary call to 70. Do not infer runtime semantics from the
forced demonstration, or assume `constexpr` alone forces evaluation.

[Editable span](https://godbolt.org/z/474oesPvP) ·
[Boundary comparison](https://godbolt.org/z/5he7s1xKq).

## A union's hidden discriminator

For tested scalar alternatives, probing the active member succeeds and probing
the inactive member fails:

```cpp
union Number {
    int i;
    double d;

    consteval int index() const {
        if (__builtin_constant_p(i)) return 0;
        if (__builtin_constant_p(d)) return 1;
        return -1;
    }
};
```

The full demonstration includes visitation with a common result type, switching
alternatives, and distinguishing integer zero from floating zero. It occupies
eight bytes on the tested targets and passed both compilers at C++20 `-O0` and
`-O2`. An initial throwing fallback in the non-template `index()` triggered
Clang's invalid-constexpr analysis; the saved version returns −1.

The active member is evaluator metadata. This ingredient has
[documented prior art](01-context-and-decisions.md#provenance-and-corrections).
It does not make every payload type reliably probeable.

[Editable union](https://godbolt.org/z/1bazeoePG).

## First owning vector: two probes per slot

The original integer container allocated union slots with an `unsigned char
empty` member or an `int value` member. Only `slot*` was stored in the container.

| Slot state | Probe `value` | Probe `empty` |
| --- | ---: | ---: |
| Occupied | 1 | 0 |
| Unused capacity | 0 | 1 |
| One-past allocation | 0 | 0 |

`size()` scanned the active-value prefix. `capacity()` scanned while either
member was readable. Assignment changed the active member on push/pop. The
original report records successful empty construction, reservation, growth from
4 to 8, pop, mutation, move construction, clear/reuse, and `INT_MIN`/`INT_MAX`
checks on both compilers at C++20 `-O0`/`-O2`.

That complete original integer source is in
[its Godbolt link](https://godbolt.org/z/6f867K31h). **The scratch file named
`one-pointer-vector.cpp` was later overwritten with a compact template.** Its
loose old compiler responses cannot all be bound to its final text by filename.
Use the original Godbolt link for that source and the root file for the selected
template. Its C++23 checks were separately tied to its source hash.

A rejected capacity shortcut probed `(p + n) - p`. Both compilers could fold
that arithmetic beyond the allocation; it did not locate its end.

## Art pass: probe only a scalar marker

Templating the direct value probe was insufficient: aggregate recognition
differs between compilers. The selected [root implementation](../../tricks/one-pointer-vector/one-pointer-vector.cpp)
instead uses:

```cpp
union slot { char empty = 0; T value; };
slot* p = nullptr;
```

The occupied prefix has inactive `empty` members. It is followed by **at least
one active empty marker**, including when usable capacity is full. `size()`
stops at that marker; `capacity()` scans the remaining active markers until the
allocation ends and subtracts the reserved slot. Without that slot, an inactive
marker and an out-of-bounds probe both look false to the size loop.

Null means no allocation, size 0, capacity 0. Growth at size `n` allocates
`2*n + 2` slots, giving usable capacities 1, 3, 7, ... . The copy loop
`for (int i = n; i--;) q[i] = p[i];` transfers active union states. Push assigns
a union with active `value`; pop saves the value and uses
`return p[n].empty = 0, x;` to reactivate the marker before returning it.

No value of `T` is reserved. There is one pointer in the vector object, one
shared union storage area per slot, and one extra reserved slot per allocation.
For tested `int` slots, the slot size equals `sizeof(int)`.

### Evolution evidence, including unsuccessful runs

- `vector-art.cpp` is the earlier direct-value
  template. Its retained scalar checks pass both compilers at C++20 `-O0`/`-O2`.
- `probe-types.cpp` explored aggregate/copy/pointer
  recognition. Its ordinary global initializer is not a universally forced
  constant-evaluation truth table; preserve the evaluation context.
- `vector-sentinel.cpp` puts `T value` first
  and eagerly allocates one marker slot. Retained Clang `-O0`/`-O2` runs pass;
  GCC default-limit runs timed out. A GCC run with
  `-fconstexpr-loop-limit=64` passed. This is an observation, not a proven
  explanation of the timeouts or of member-order sensitivity.
- `sentinel-isolate.cpp` reduced those cases.
  GCC accepted the saved limited-loop run. Clang rejected that GCC-specific
  command-line option before evaluating the program; that failure says nothing
  about the reduced source's semantics.
- `vector-validation.cpp` is the exact
  initial root source: marker first, null initial pointer, full checks. It
  passed both compilers at C++20 `-O0`/`-O2`, then was rechecked in C++23. The
  compact [Godbolt sketch](https://godbolt.org/z/MscKzxdjf) has shorter checks
  and C++20 settings.

### What the selected sketch promises

Checks cover `int`, `double`, `bool`, a small aggregate, pointers including null,
and a class with a converting constructor. The union operations expect suitable
trivially copyable/destructible types; this is not an implementation for all `T`.
Keep the owning vector uncopied, pop only when nonempty, and index live elements.
Its implicit copy would duplicate ownership; the art pass did not add a general
ownership API. Do not silently reintroduce the older container's move/reserve/
clear interface in explanations.

Queries scan linearly. The allocation is an array of union slots, not a `T[]`
supporting a standard contiguous `data()` interface. All allocations are freed
within their constant evaluation. The evaluator still pays for bounds and
lifetime metadata; this is not a runtime memory-compression claim.

## The compile-time cat detour

The self-contained adaptation uses `#embed __FILE__` and a computed
`static_assert` message with `data()` and `size()` to print its own source.
Both compilers produced the source diagnostic at `-std=c++2c`, `-O0`/`-O2`;
**exit 1 is its intended output**. Clang also warned about `#embed` as an
extension. The original repository example depended on a remote fmt include
that the compile API did not resolve, so a self-contained version was used.

This was not promoted into the C++23 root set.
[Godbolt cat](https://godbolt.org/z/ja4js9bj7).
