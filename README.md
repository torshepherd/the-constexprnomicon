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

Each file stands alone and carries its own `static_assert` checks. All three use
C++23 with compiler extensions; none uses reflection. These are experiments in
compiler behavior, with the relevant assumptions recorded in
[the notes](NOTES.md).

## Read the spells

The [notes](NOTES.md) explain the mechanisms, limitations, known related work,
and link to editable Godbolt examples.

To check a file locally with the listed compiler:

```sh
g++-16 -std=c++23 -O2 -c one-pointer-vector.cpp -o /tmp/one-pointer-vector.o
clang++-22 -std=c++23 -O2 -c empty-bits.cpp -o /tmp/empty-bits.o
g++-16 -std=c++23 -O2 -c cache-counter.cpp -o /tmp/cache-counter.o
```

Compiler executable names depend on your installation. Recorded checks passed
at both `-O0` and `-O2` on the versions above; changes to evaluators or their
limits can change the outcome.
