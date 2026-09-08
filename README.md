# The Constexprnomicon

A grimoire of forbidden compile-time C++, conjured by human and machine.

Small spells that reward contemplation. The aim is to reduce each trick to its
essence: concise code, surprising behavior, and just enough explanation to see
what the compiler is being persuaded to do.

Created by Tor Shepherd in collaboration with OpenAI's Codex. This is a new,
independent companion to [sorcery-cpp](https://github.com/torshepherd/sorcery-cpp),
which remains a historical artifact of the pre-AI era. Tor's
[Static Antics](https://github.com/torshepherd/static_antics) provided the starting
point for these experiments.

## Spells

| Spell | The trick | Tested compiler |
| --- | --- | --- |
| [One-pointer vector](one-pointer-vector.cpp) | The evaluator remembers size and capacity through union lifetimes. | GCC 16.2, Clang 22.1.0 |
| [Empty bits](empty-bits.cpp) | An empty class carries 64 writable logical bits in its base lifetimes. | Clang 22.1.0 |
| [Cache counter](cache-counter.cpp) | GCC's constexpr memoization cache becomes the counter's storage. | GCC 16.2 |
| [Cache memory](cache-memory.cpp) | An append-only cache becomes a writable dictionary across constant evaluations. | GCC 13.3, GCC 16.2 |
| [Cache vector](cache-vector.cpp) | An empty, const global handle can push, pop, and overwrite elements in compiler storage. | GCC 13.3, GCC 16.2 |
| [Copy gate](copy-gate.cpp) | A copy constructor sorts a template argument; an unconstrained overload accepts only already-sorted words. | GCC 13.3 |
| [Last rites](last-rites.cpp) | Temporary destructors run backpropagation: the semicolon differentiates an expression. | GCC 13.3, GCC 16.2 |

Each file stands alone and carries its own `static_assert` checks. The spells use
C++23 and depend on compiler behavior; none uses reflection. These are experiments in
compiler behavior, with the relevant assumptions recorded in
[the notes](NOTES.md).

## Advanced applications

[The sortable cache vector](advanced/cache-vector.cpp) keeps the small integer
spells above intact and adds codecs, live proxies, and random-access iterators.
It sorts integers, owning strings, and field-encoded records in compiler storage:

```cpp
static_assert([] {
    auto v = numbers; // A local empty handle, not a copy of the elements.
    std::ranges::sort(v);
    return std::ranges::is_sorted(v);
}());
```

Checked on GCC 13.3.0 at `-O0` and `-O2`, with C++23 and
`-fconstexpr-cache-depth=64`. The local handle and deeper cache are essential to
the tested algorithm behavior. Read the [advanced notes](NOTES.md#sortable-cache-vector)
before trying it; this is not an ordinary runtime container.

## Read the spells

The [notes](NOTES.md) explain the mechanisms, limitations, known related work,
and link to editable Godbolt examples.

To check a file locally with the listed compiler:

```sh
g++-16 -std=c++23 -O2 -c one-pointer-vector.cpp -o /tmp/one-pointer-vector.o
clang++-22 -std=c++23 -O2 -c empty-bits.cpp -o /tmp/empty-bits.o
g++-16 -std=c++23 -O2 -c cache-counter.cpp -o /tmp/cache-counter.o
g++-16 -std=c++23 -O2 -c cache-memory.cpp -o /tmp/cache-memory.o
g++-16 -std=c++23 -O2 -c cache-vector.cpp -o /tmp/cache-vector.o
g++-13 -std=c++23 -O2 -c copy-gate.cpp -o /tmp/copy-gate.o
g++-16 -std=c++23 -O2 -c last-rites.cpp -o /tmp/last-rites.o
```

Compiler executable names depend on your installation. The notes record each
spell's checked versions and optimization levels; changes to evaluators or
their limits can change the outcome.
