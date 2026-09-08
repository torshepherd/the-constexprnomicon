# The Constexprnomicon

A grimoire of forbidden compile-time C++, conjured by human and machine.

Small spells that reward contemplation. The aim is to reduce each trick to its
essence: concise code, surprising behavior, and just enough explanation to see
what the compiler is being persuaded to do.

The collection has two lines of inquiry:

- **[Standalone tricks](#standalone-tricks):** small constructions with surprising
  behavior, such as an empty integer or a SAT oracle made of overload ambiguity.
- **[Roughly-Turing-complete features](MACHINES.md):** investigations of whether
  one precisely defined language mechanism supplies selection, memory, and
  recurrence. Demonstrated ingredients and open machine claims are tracked
  separately.

Created by Tor Shepherd in collaboration with OpenAI's Codex. This is a new,
independent companion to [sorcery-cpp](https://github.com/torshepherd/sorcery-cpp),
which remains a historical artifact of the pre-AI era. Tor's
[Static Antics](https://github.com/torshepherd/static_antics) provided the starting
point for these experiments.

## Standalone tricks

Each entry demonstrates a particular capability. Inclusion here is not a claim
that its underlying mechanism can express arbitrary programs.

| Spell | The trick | Tested compiler |
| --- | --- | --- |
| [One-pointer vector](one-pointer-vector.cpp) | The evaluator remembers size and capacity through union lifetimes. | GCC 16.2, Clang 22.1.0 |
| [Empty bits](empty-bits.cpp) | An empty class carries 64 writable logical bits in its base lifetimes. | Clang 22.1.0 |
| [Cache counter](cache-counter.cpp) | GCC's constexpr memoization cache becomes the counter's storage. | GCC 16.2 |
| [Cache memory](cache-memory.cpp) | An append-only cache becomes a writable dictionary across constant evaluations. | GCC 13.3, GCC 16.2 |
| [Cache vector](cache-vector.cpp) | An empty, const global handle can push, pop, and overwrite elements in compiler storage. | GCC 13.3, GCC 16.2 |
| [Copy gate](copy-gate.cpp) | A copy constructor sorts a template argument; an unconstrained overload accepts only already-sorted words. | GCC 13.3 |
| [Last rites](last-rites.cpp) | Temporary destructors run backpropagation: the semicolon differentiates an expression. | GCC 13.3, GCC 16.2 |
| [False idols](false-idols.cpp) | Overload ambiguity decides SAT, even though every atomic constraint is literally true. | GCC 13.3, GCC 16.2, Clang 22.1.0 |

Each file stands alone and carries its own `static_assert` checks. The spells use
C++20 or C++23; none uses reflection. Compiler-specific assumptions and checked
language versions are recorded in [the notes](NOTES.md).

False idols contains no function bodies. Named concepts provide symbolic atoms,
the compiler's constraint ordering answers the SAT question, and a final
`requires` expression reads whether the call is ambiguous. Its ordinary Boolean
value and its satisfiability answer can disagree:

```cpp
static_assert(formula<int>);       // Every atom evaluates to true.
static_assert(!satisfiable<int>);  // The encoded formula has no solution.
```

### Applications of standalone tricks

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

## Roughly-Turing-complete features

Can a language feature become a programming language of its own? Our criterion
is selection, memory, and recursion or iteration within an explicit set of
allowed operations. Reading the final answer is distinguished from using another
mechanism to carry intermediate computation.

The [machine investigations](MACHINES.md) track CTAD, hidden copy plus overload
resolution, and ambiguity-driven computation. **No complete isolated machine is
currently demonstrated by the examples tracked there.** False idols establishes
a finite SAT oracle; whether ambiguity can also drive recurrence and evolving
state is a separate, open question.

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
g++-16 -std=c++20 -O2 -fsyntax-only false-idols.cpp
```

Compiler executable names depend on your installation. The notes record each
spell's checked versions and optimization levels; changes to evaluators or
their limits can change the outcome.
