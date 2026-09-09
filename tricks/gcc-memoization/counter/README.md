# Cache counter

[Source](counter.cpp) · [Godbolt](https://godbolt.org/z/j6d7b6nEn)

An uncached `slow(key, 520)` exceeds the tested default recursion budget.
`__builtin_constant_p` converts that failure into zero. Evaluating
`slow(key, 500)` fits and seeds GCC's constexpr memoization cache; a later
520-level probe can then finish through cached subcalls.

Warm keys form an observable set. `next()` scans from zero to the first cold
key, warms it, and returns it. No C++ global object holds the count.

The unused default argument gives `next()` itself a different cache key on
different source lines. It does not supply the count. Reusing an explicit
argument demonstrates that the wrapper is memoized too: 4, 4, then 5.
Calls on the same line, repeated execution of one call site, and line-number
collisions across files can reuse that key.

[The smaller cache probe](https://godbolt.org/z/v3MT7f8xG) asks whether
`f(520)` is constant before and after evaluating `f(500)`:

| Compiler and additional options | Before | After |
| --- | ---: | ---: |
| GCC 16.2 | 0 | 1 |
| GCC 16.2, `-fconstexpr-cache-depth=0` | 0 | 0 |
| GCC 16.2, `-fconstexpr-depth=1024` | 1 | 1 |
| Clang 22.1.0 | 0 | 0 |

These controls were run at `-std=c++23 -O2`. GCC documents
[its recursion and cache controls](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html).
The constants depend on those limits. Disabling caching or increasing the
recursion budget breaks the counter's mechanism; Clang does not pass the
counter's increment assertions.

## Try it

Recorded compiler evidence: **GCC 13.3 and 16.2, default constexpr depth/cache settings**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++-16 -std=c++23 -O2 -c tricks/gcc-memoization/counter/counter.cpp -o /tmp/cache-counter.o
```

See [shared provenance](../../../docs/PROVENANCE.md) for known ingredients and related work.

The [cleanup check](../../../.agents/notes/16-repository-cleanup.md#verification)
also passes GCC 13.3.0 at C++23 `-O2`, with default constexpr limits.
