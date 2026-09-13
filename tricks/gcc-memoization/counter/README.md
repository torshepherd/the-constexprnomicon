# Cache counter

**GCC's constexpr cache remembers the count.** The same mechanism works with
either a builtin probe or a constant-expression requirement.

| Version | Source | Call identity |
| --- | --- | --- |
| Builtin scan | [counter.cpp](counter.cpp) · [Godbolt](https://godbolt.org/z/j6d7b6nEn) | Ordinary argument, defaulting to the caller's line |
| Template scan | [counter-requires.cpp](counter-requires.cpp) · [Godbolt, third editor](https://godbolt.org/z/on8WTjYKq) | Explicit `Unique` template argument |

## Make cache membership observable

Both versions start with the same recursive function:

```cpp
constexpr int slow(int key, int depth = 520) {
    return depth ? slow(key, depth - 1) : key;
}
```

An uncached `slow(key, 520)` exceeds the tested default recursion budget.
Evaluating `slow(key, 500)` fits and seeds GCC's memoization cache; a later
520-level probe can finish through cached subcalls. Warm keys form an
observable set. Find the first cold key, warm it, and return it. No C++ global
object holds the count.

## Probe with `__builtin_constant_p`

The original version scans ordinary integer values:

```cpp
consteval int next(int = __builtin_LINE()) {
    int n = 0;
    while (__builtin_constant_p(slow(n))) ++n;
    return slow(n, 500);
}
```

The builtin converts failed constant evaluation into zero. The unused default
argument gives `next()` itself a different cache key on different source lines;
it does not supply the count. Reusing an explicit argument demonstrates that
the wrapper is memoized too: 4, 4, then 5. Calls on the same line, repeated
execution of one call site, and line-number collisions can reuse that key.

## Probe with `requires` and `if constexpr`

The alternate version uses no builtins:

```cpp
template<int Unique, int N = 0> consteval int next() {
    if constexpr (N == 8) return -1; // Bound the experiment.
    else if constexpr (requires {
        typename std::integral_constant<int, slow(N)>;
    })
        return next<Unique, N + 1>();
    else return slow(N, 500);
}
```

Forming `std::integral_constant` requires evaluating its second template
argument. If that evaluation fails during substitution, the
[type requirement](https://eel.is/c++draft/expr.prim.req.type) becomes false.
`if constexpr` selects the corresponding branch; template recursion replaces
iteration. A simple `requires { slow(N); }` would only check expression
formation and would not perform this constant-expression test.

The retained checks give `next<0>() == 0`, `next<1>() == 1`, and so on.
A fresh `Unique` specialization permits a fresh check; repeating `next<3>()`
returns 3 again. The bound of eight lets the raised-depth control terminate.

This changes how arguments arrive. The scan index and freshness identity are
now template arguments; an ordinary parameter does not become a template
argument just because its enclosing function is `consteval`. The
[dictionary](../memory/) and [vectors](../vector/) still use ordinary keys,
values, and loop-driven accesses. Their complete interfaces have not been
rewritten using this technique. The alternate counter remains a GCC cache
exploit, even though its source uses ordinary language/library constructs.

## What the controls establish

The [smaller builtin probe](https://godbolt.org/z/v3MT7f8xG) asks whether
`f(520)` is constant before and after evaluating `f(500)`:

| Compiler and additional options | Before | After |
| --- | ---: | ---: |
| GCC 16.2 | 0 | 1 |
| GCC 16.2, `-fconstexpr-cache-depth=0` | 0 | 0 |
| GCC 16.2, `-fconstexpr-depth=1024` | 1 | 1 |
| Clang 22.1.0 | 0 | 0 |

The template counter's independent controls agree with that mechanism:

| GCC 16.2 configuration | Template counter result |
| --- | --- |
| Default limits | 0, 1, 2, 3; repeated identity gives 3; fresh identity gives 4 |
| Cache disabled, `NO_CACHE` | Distinct identities both return 0 |
| Depth 1024, `DEEP_LIMIT` | Every tested cold key is evaluable; bounded scan returns -1 |

These controls pass their expectations at C++23/O2. Clang 22.1.0 returns zero
repeatedly and fails the increment assertions in both implementations.
GCC documents [the depth/cache options](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html).
The constants depend on those limits. Do not raise the depth for the unbounded
builtin counter: making every key appear warm breaks its stopping condition.
Neither `std::is_constant_evaluated()` nor `std::is_within_lifetime` measures
whether a recursive call fits the evaluator's current budget.

## Try both

The builtin source passes GCC 13.3 at C++23/O2 and GCC 16.2 at C++23/O0/O2.
The template source and its controls were verified on GCC 16.2, C++23/O2. Run from the repository
root with the matching compiler installed:

```sh
g++-16 -std=c++23 -O2 -c tricks/gcc-memoization/counter/counter.cpp -o /tmp/cache-counter.o
g++-16 -std=c++23 -O2 -c tricks/gcc-memoization/counter/counter-requires.cpp -o /tmp/cache-counter-requires.o

# Controls for the bounded template version only:
g++-16 -std=c++23 -O2 -fconstexpr-cache-depth=0 -DNO_CACHE -fsyntax-only tricks/gcc-memoization/counter/counter-requires.cpp
g++-16 -std=c++23 -O2 -fconstexpr-depth=1024 -DDEEP_LIMIT -fsyntax-only tricks/gcc-memoization/counter/counter-requires.cpp
```

[Research evidence](../../../.agents/notes/22-constantness-alternatives.md) ·
[Earlier GCC 13.3 check](../../../.agents/notes/16-repository-cleanup.md#verification) ·
[Shared provenance](../../../docs/PROVENANCE.md)
