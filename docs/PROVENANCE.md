# Provenance and related work

The constructions were developed and tested during the conversation. A targeted
prior-art search found no exact match for the cache counter or the empty-base
bit store; this is not an established claim of first discovery.

[Filip Roséen's 2015 constexpr counter](https://refp.se/articles/constexpr-counter)
uses friend injection. It is related stateful metaprogramming with a different
storage mechanism.

[P2641R4, section 3.5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2641r4.html#implementation-experience)
already documents using `__builtin_constant_p` for lifetime/active-union
detection, crediting Johel Ernesto Guerrero Peña and Ed Catmur. That ingredient,
including the active-union probe used by the vector, is known prior work.

[GCC's builtin documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
describes constant recognition, not a universal dereferenceability test.
The probes in these spells are deliberately narrow; arbitrary expressions,
especially ones with side effects, need their own investigation.

Each [trick’s README](../README.md#standalone-tricks) records its own compiler
evidence, assumptions, and additional sources. Research history is preserved in
the [working notes](../.agents/notes/README.md).
