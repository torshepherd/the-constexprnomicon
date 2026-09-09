# Compiler state as storage

These experiments use C++23 and no reflection. The selected root spells have
their own checks; the other programs below are research branches documented
here. Scratch filenames are historical identifiers, not additional repo files.

## GCC's memoization counter

The [root counter](../../tricks/gcc-memoization/counter/counter.cpp) consists of a recursive function and a
scan of keys whose deep call has become recognizable:

```cpp
constexpr int slow(int key, int depth = 520) {
    return depth ? slow(key, depth - 1) : key;
}

consteval int next(int = __builtin_LINE()) {
    int n = 0;
    while (__builtin_constant_p(slow(n))) ++n;
    return slow(n, 500);
}
```

At the tested default depth limit, an uncached 520-deep invocation fails the
probe. A 500-deep invocation succeeds and warms GCC's constexpr cache. A later
520-deep probe can finish through cached subcalls. Warm keys form a growing set;
`next()` finds the first absent key and warms it.

There is no mutable global C++ object, template, friend injection, or
`__COUNTER__` holding the state. The local `n` is a scan position. The unused
line-number argument gives the `next` wrapper distinct memoization keys; it is
not the returned count. The checks deliberately demonstrate wrapper caching:
four calls on separate lines yield 0, 1, 2, 3, then explicit arguments 9000,
9000, 9001 yield 4, 4, 5.

The complete root file passed GCC 16.2 at `-std=c++23 -O0` and `-O2`. Clang
22.1.0 failed the increment assertions, reporting repeated zero. A separate
set probe warmed keys 7 and 8 independently;
GCC passed, Clang failed the expected-presence checks.

The smaller cache probe asks whether `f(520)` is
constant before and after an ordinary constant evaluation of `f(500)`:

| C++23 `-O2` configuration | Before | After |
| --- | ---: | ---: |
| GCC 16.2, defaults | 0 | 1 |
| GCC, `-fconstexpr-cache-depth=0` | 0 | 0 |
| GCC, `-fconstexpr-depth=1024` | 1 | 1 |
| Clang 22.1.0, defaults | 0 | 0 |

GCC documents default recursion depth 512 and cache depth 8 in
[its dialect options](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html).
The controls support the cache explanation. Changing these limits can break the
scan's premise; raising the depth can make cold keys appear warm. Do not run an
unbounded counter experiment under arbitrary limits and assume it terminates.

Calls on the same source line, repeated execution of one call site, and equal
line numbers across files can reuse the wrapper key. This is not a robust
cross-translation-unit counter. Prior-art context is in
[the decisions](01-context-and-decisions.md#provenance-and-corrections).

[Editable counter](https://godbolt.org/z/j6d7b6nEn) ·
[Editable cache control](https://godbolt.org/z/v3MT7f8xG).

## Empty bits on Clang

The [root empty bit store](../../tricks/empty-bits/empty-bits.cpp) inherits 64 distinct empty base
types, `bit<I>`. Each has a trivial non-static `alive()` method returning true.
The lifetime of base `I` represents logical bit `I`:

- A probe of `base->alive()` distinguishes live from dead on the tested Clang.
- `std::destroy_at` clears a bit; `std::construct_at` sets it.
- `get()` ORs the 64 probe results at their respective bit positions.
- `set(n)` destroys currently live bases, then reconstructs those selected by
  the ordinary function argument `n`.
- The destructor reconstructs all bases before their implicit destruction.

`integer` satisfies both `sizeof(integer) == 1` and `std::is_empty_v<integer>`
on the tested target. Its template arguments specify positions, not the payload.
Two instances of the same type store different values, and each can change
during one constant evaluation.

The full Clang 22.1.0 source passed at C++23 `-O0`/`-O2`: size/emptiness, initial
and changed 64-bit patterns, two independent objects, sixteen generated
patterns, repeated zero/all-one writes, and final destruction. GCC 16.2 rejected
the experiment with destruction-outside-lifetime diagnostics.

The isolated empty-method probe uses a method
returning zero without reading a field:

| Probe during forced evaluation, C++23 `-O2` | GCC | Clang |
| --- | ---: | ---: |
| Active union member | 1 | 1 |
| Inactive union member | 1 | 0 |
| Deleted object | 1 | 0 |

Thus the trivial call is not a cross-compiler liveness primitive. The empty
store also depends on Clang's handling of replacement of empty base lifetimes,
including potentially overlapping subobjects. Acceptance is not a proof of
standard lifetime semantics. Its payload lives in evaluator bookkeeping; copying
the wrapper's bytes does not copy that state. Keep it uncopied and confined to
constant evaluation. An empty complete object still occupies a byte; the claim
is logical state without data members, not a physical zero-byte complete object.

[Editable empty bits](https://godbolt.org/z/cxqejscT1).

## The nested-union byte

`cursed-byte.cpp` tried another way to carry more
state than the layout suggests:

```cpp
#include <memory>

template<int N> union byte {
    unsigned char digit;
    byte<N - 1> next;
    constexpr byte(unsigned n) : digit(n) {
        if (n >= 256) std::construct_at(&next, n - 256);
    }
    constexpr unsigned get() const {
        return __builtin_constant_p(digit) ? digit : 256 + next.get();
    }
};
```

The base case and the meaningful checks were:

```cpp
template<> union byte<0> {
    unsigned char digit;
    constexpr byte(unsigned n) : digit(n) {}
    constexpr unsigned get() const { return digit; }
};
static_assert(sizeof(byte<100>) == 1);
static_assert(byte<100>(12345).get() == 12345);
static_assert(byte<100>(42).get() == 42);
int demonstration() { return byte<100>(12345).get(); }
```

Each active `next` contributes
256; the active terminal digit contributes the remainder. The template bounds
the representable depth, while the selected active-member path carries the
value. This sketch does not check overflow beyond its depth bound.

**Both GCC 16.2 and Clang 22.1.0 accepted** the C++23 `-O2` assertions that
`sizeof(byte<100>) == 1`, and that values 12345 and 42 round-trip. This was not a
failed compile-time experiment; it simply was not selected for the root trio.
The saved diagnostics are empty and both compiler exit codes are zero.

The same file has an ordinary `demonstration()` calling `byte<100>(12345).get()`.
Both emitted assembly returning **57**, the low byte of 12345. Consequently,
passing those `static_assert`s does not establish ordinary-call semantics. No
separate investigation established the optimizer's precise reason for that
difference. Preserve both facts before developing this sketch further; do not
present its ordinary call as returning 12345.

## Measuring recursion headroom

`cursed-depth.cpp` binary-searches the largest
recursive `dive(mid)` the builtin recognizes. Its compact update is
`(__builtin_constant_p(dive(mid)) ? lo : hi) = mid;`.

| Forced result, C++23 `-O2`, search ceiling 1024 | Clang | GCC |
| --- | ---: | ---: |
| Direct call to `room()` | 510 | 511 |
| One wrapper | 509 | 511 |
| Two wrappers | 508 | 511 |
| Three wrappers | 507 | 511 |

Clang observes remaining call-stack budget without the caller passing a depth.
GCC's cache spoils a straightforward depth interpretation. An earlier search
with ceiling 600 produced 599 on GCC; it appears in the
budget probe. There the direct/deeper results
were Clang 510/489 and GCC 599/599.

`room` is `constexpr`, not `consteval`: forcing a no-argument immediate call
inside its wrapper would evaluate it before the intended outer stack exists.
Results depend on limits, search order, and evaluator history, not just source
nesting. [Clang's documented limits](https://clang.llvm.org/docs/UsersManual.html#controlling-implementation-limits)
provide the background.

[Editable depth comparison](https://godbolt.org/z/TcsWEdrG1).

## Turning evaluator failure into fallback

The budget probe includes:

```cpp
consteval int attempt(auto f) {
    return __builtin_constant_p(f()) ? f() : -1;
}
```

Both compilers passed C++23 `-O2` checks returning 42 for shallow recursion and
−1 for depth 1000, endless recursion, and an invoked throwing lambda. This
observes resource-bounded failure, not the mathematical termination of arbitrary
programs. A terminating computation can exceed a budget too. The probe and
subsequent `f()` are separate evaluations; this is not an evaluate-once facility.

## Side effects are not a transaction

The initial probe started `x = 7`, called a preincrementing `bump(x)` inside the
builtin, and returned `known * 100 + x`. At both C++20 optimization levels it
produced GCC 108 and Clang 7.
[Editable comparison](https://godbolt.org/z/PPzz6K86r).

The later transaction probe started `x = 1`,
added 10 inside the probed call, optionally threw, and encoded the result the
same way:

| C++23 `-O2` | GCC | Clang |
| --- | ---: | ---: |
| Successful probed call | 111 | 1 |
| Call mutates then throws | 11 | 1 |

GCC retained the mutation even after the failed probe. Clang returned zero from
the probe and left `x` unchanged in both cases. This conflicts with treating
[GCC's documented non-evaluation behavior](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
as a dependable transaction model for these examples. No bug was filed or
diagnosed to completion. The cache counter relies on evaluator memoization,
not mutation of C++ objects inside a probe.
