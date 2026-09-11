# Phase shift

One unchanged struct decomposes into two elements above a declaration and three
below it. The earlier function keeps its two-element interpretation even when
called after the declaration.

[The complete spell](phase-shift.cpp) is C++17. This is a small declaration-order
curiosity, not invisible storage, mutable compiler state, or a computational
universality claim. It works outside constant evaluation too.

## 1. Give the object exactly two members

```cpp
struct coordinate { int x; int y; };
```

Nothing later changes this definition. The checked implementations give it the
size of two `int`s; the spell asserts that layout observation.

## 2. Announce a tuple adapter, but do not define it

```cpp
namespace std {
template<> struct tuple_size<coordinate>;
}
```

This explicitly declared specialization is incomplete. Such a declaration does
not implicitly instantiate a definition. This use of an incomplete specialization
is permitted by [the specialization rules](https://eel.is/c++draft/temp.expl.spec).

For a non-array class, structured bindings select the tuple protocol only when
`tuple_size<T>` is complete and has a member named `value`. Otherwise they use
member decomposition. [Structured-binding rules](https://eel.is/c++draft/dcl.struct.bind).

## 3. Bind the real members

Before completing the adapter, define an ordinary function:

```cpp
constexpr int before() {
    coordinate point{10, 20};
    auto [x, y] = point;
    return 100 * x + y;
}
```

This produces `1020`. The function body is not a template waiting for a later
argument type.

## 4. Complete the adapter with a different shape

```cpp
namespace std {
template<>
struct tuple_size<coordinate> : integral_constant<size_t, 3> {};

template<> struct tuple_element<0, coordinate> { using type = int; };
template<> struct tuple_element<1, coordinate> { using type = int; };
template<> struct tuple_element<2, coordinate> { using type = int; };
}

template<std::size_t I>
constexpr int get(coordinate point) {
    int values[]{point.y, point.x, point.x + point.y};
    return values[I];
}
```

Later bindings now select tuple decomposition and find our global `get` by
argument-dependent lookup. They expose `y`, `x`, and a computed sum. The protocol
does not add a third data member. [Structured-binding rules](https://eel.is/c++draft/dcl.struct.bind).

These are standard-library class-template customizations for a program-defined
type, not arbitrary new declarations in `std`. The completed size trait derives
from the required `integral_constant`. See [namespace std](https://eel.is/c++draft/namespace.std)
and [tuple helpers](https://eel.is/c++draft/tuple.helper).

## 5. Ask the same type a different question

```cpp
constexpr int after() {
    coordinate point{10, 20};
    auto [y, x, sum] = point;
    return 10000 * y + 100 * x + sum;
}

static_assert(before() == 1020);
static_assert(after() == 201030);
```

| Binding appears | Adapter visible there | Elements exposed |
| --- | --- | --- |
| In `before` | Incomplete | `x`, `y` |
| In `after` | Complete, size three | `y`, `x`, computed sum |

Both calls appear after completion. Their order is irrelevant: this is a change
between declaration sites, not an object changing shape during execution.

## The follow-up: bootstrap the adapter using its old interpretation

[bootstrap.cpp](experiments/bootstrap.cpp) replaces the size definition with:

```cpp
template<> struct tuple_size<coordinate>
    : integral_constant<size_t, [] {
          auto [x, y] = coordinate{1, 2};
          return x + y;
      }()> {};
```

This code goes inside `namespace std`, following the forward declaration. While
computing its base-class argument, the specialization is still incomplete, so
the lambda binds two real members. It supplies the value three; completing the
definition then makes subsequent bindings demand three elements. The lambda is
in the base-specifier, not a complete-class context within the member-specification.
[Class completeness](https://eel.is/c++draft/class.mem.general).

In other words: **the adapter can use the interpretation that defining the
adapter will disable.** This version needs C++20 for a lambda in a template
argument, and currently has local GCC evidence only. It computes `1 + 2`; it
does not automatically count fields or provide generic reflection.

## Controls and limits

- [references.cpp](experiments/references.cpp) passes one object through both
  functions. Address checks establish that both views alias its real members.
  In the later view, `get<2>` returns a sum by value: that binding is a snapshot,
  not a live formula. The tuple binding's temporary lasts through the binding's
  scope. [Structured-binding rules](https://eel.is/c++draft/dcl.struct.bind).
- The same file deliberately fails with `EARLY_THREE` (three names before the
  switch), `LATE_TWO` (two afterward), or `OMIT_GET` (tuple protocol selected, but
  no matching accessor). A broken adapter does not trigger member fallback.
- [late-template.cpp](experiments/late-template.cpp) defines a generic lambda
  early but instantiates it only after completion. Its dependent binding sees
  three elements. Source location alone is not the rule for templates.
- [rejected-frozen-const.cpp](experiments/rejected-frozen-const.cpp) fails: an
  earlier const binding did not freeze a two-element const view for later use.
- [rejected-frozen-nondependent.cpp](experiments/rejected-frozen-nondependent.cpp)
  fails: an unused template parameter did not preserve an early nondependent
  two-element interpretation when instantiated later. Changing the meaning of
  a nondependent construct between hypothetical and actual instantiation can be
  ill-formed with no required diagnostic. [Template resolution](https://eel.is/c++draft/temp.res.general).

One specialization can be completed once, not repeatedly rewritten. Do not
extrapolate this into mutable traits or give one template specialization
conflicting meanings at different instantiation points or across translation
units. [Points of instantiation](https://eel.is/c++draft/temp.point).

## Reproduce

Run from the repository root:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/phase-shift/phase-shift.cpp
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/phase-shift/experiments/bootstrap.cpp
g++ -std=c++17 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/phase-shift/experiments/references.cpp
g++ -std=c++17 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/phase-shift/experiments/late-template.cpp
```

For each negative control, compile `references.cpp` with C++17/O0 and one of
`-DEARLY_THREE`, `-DLATE_TWO`, or `-DOMIT_GET`; expect rejection. Compile each
`rejected-*.cpp` separately with C++20/O0; expect rejection. The const experiment
also rejects with `-DOMIT_EARLY`. Keep the same warnings and pedantic flags.

| Source | Compiler | Checked modes | Outcome |
| --- | --- | --- | --- |
| Main spell | GCC 13.3, local | C++17, O0/O2 | Accepted |
| Main spell | GCC 16.2, Compiler Explorer `g162` | C++17, O2 | Accepted, no diagnostics |
| Main spell | Clang 22.1.0, Compiler Explorer `clang2210` | C++17, O2 | Accepted, no diagnostics |
| Bootstrap | GCC 13.3, local | C++20, O0/O2 | Accepted |
| References and late template | GCC 13.3, local | C++17, O0/O2 | Accepted |
| Three macro controls | GCC 13.3, local | C++17, O0 | Rejected for the intended reasons |
| Rejected template/const probes | GCC 13.3, local | C++20, O0 | Rejected as described |

All rows used `-Wall -Wextra -pedantic-errors`; local compile-only checks also
used `-fsyntax-only`. An argc-dependent runtime smoke test of the reference
functions also passed at GCC 13.3/C++17/O0. Exact source identity and reproduction
details are in [the working notes](../../.agents/notes/19-phase-shift.md).
Compiler acceptance is evidence, not an ISO conformance proof.

## Provenance

Developed by Tor Shepherd and OpenAI's Codex during the post-Astral search and
its September 10–11 follow-up. These are known language rules composed into an
independently derived example; no historical-priority claim is made. The full
[search record](../../.agents/notes/18-out-of-left-field-search.md) preserves
the weaker and rejected directions as well.
