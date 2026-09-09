# Seance

[Source](seance.cpp) · [Research and controls](../../.agents/notes/15-seance.md)

**A compile-time check can leave a runtime ghost, even when the concept it checks
is false.** On the tested GCC and Clang configurations, this complete program
prints `boo` when you run it:

```cpp
#include <cstdio>

template<class T>
inline int ghost = std::puts("boo");

template<class T>
auto summon() { return ghost<T>; }

template<class T>
concept haunted = requires {
    summon<T>();
    typename T::missing;
};

static_assert(!haunted<int>);

int main() {}
```

The `main` function is empty. Nobody calls `summon<int>()`. The concept is false
because `int` has no nested type named `missing`. Nevertheless, checking it has
changed what the executable does.

## First, an ordinary global initializer

This part alone is unsurprising:

```cpp
inline int ghost = std::puts("boo");
```

A global variable can have a runtime initializer. On implementations that
initialize it before `main`, the program prints `boo` before entering `main`.

The template version is a recipe for separate variables:

```cpp
template<class T>
inline int ghost = std::puts("boo");
```

Merely writing the recipe does not instantiate every possible `ghost<T>`.
Something has to make a particular specialization necessary.

## Then, ask for a type without calling the function

```cpp
template<class T>
auto summon() { return ghost<T>; }
```

Suppose the compiler must check whether `summon<int>()` is a valid expression.
Its return type is `auto`, so the compiler instantiates the body to deduce that
type. The body contains a read of `ghost<int>`.

That read is an **odr-use**: a use requiring the variable's definition. It brings
`ghost<int>` and its runtime initializer into the program.

The subtle distinction is that the call expression is unevaluated, but the
separate function body's `return ghost<T>` is still a potentially evaluated
expression. Instantiating a body to discover its return type does not turn all
the expressions inside that body into unevaluated operands. The standard even
includes a `decltype(f(1))` example that instantiates an `auto`-returning function
for this reason. See [return-type deduction](https://eel.is/c++draft/dcl.spec.auto)
and [odr-use](https://eel.is/c++draft/basic.def.odr).

The function is instantiated, not called. The printing happens later, through
the global variable's initializer when the executable runs.

## Now reject the thing you just inspected

```cpp
template<class T>
concept haunted = requires {
    summon<T>();
    typename T::missing;
};
```

For `int`, the compiler checks these requirements in order:

1. Can it form `summon<int>()`? Yes. Deducing the return type instantiates the
   body and requires `ghost<int>`.
2. Does `int::missing` name a type? No. The concept is false.

The failure does not undo the specialization instantiated by the first check.
The executable retains the ghost. Requirement checking proceeds in lexical
order and stops once the result is determined; see
[requires-expressions](https://eel.is/c++draft/expr.prim.req.general).

## Two ways to exorcise it

**Replace `auto` with `int`:**

```cpp
template<class T>
int summon() { return ghost<T>; }
```

The compiler already knows the function's return type. The unevaluated query
does not need its body, so it does not bring in `ghost<int>`. The same
`static_assert` still passes, but the executable becomes silent.

**Or put the failure first:**

```cpp
template<class T>
concept warded = requires {
    typename T::missing;
    summon<T>();
};
```

For `int`, checking stops before reaching `summon`. Again, false and silent.
Use a fresh type or a separate program when comparing the two: an earlier query
that already instantiated a ghost cannot be undone by a later silent query.

The standalone source checks `haunted<int>` twice and `warded<double>` once. It prints
exactly one `boo` on the tested configurations. There is one initialization per
instantiated variable, not one per query.

## Rejected overloads can haunt the program too

```cpp
template<haunted T>
int choose(T);

char choose(...);

static_assert(sizeof(choose(42)) == sizeof(char));
```

In a version without the earlier `haunted<int>` assertion, merely resolving
this unevaluated call still brings in the ghost. The constrained overload is
rejected; the fallback is selected. Neither overload needs a definition because
neither is called. The controls use `std::is_same_v` to check the selected return
type exactly, without relying on integer sizes.

This is a runtime footprint of a reached compile-time check. It is not a complete
overload-resolution trace: short-circuited checks leave nothing, repeated
specializations share a variable, and initialization order does not promise the
compiler's original visitation order.

## What is actually guaranteed?

The construction uses ordinary C++20 language features, without constexpr
evaluation, friend injection, reflection, compiler builtins, or intentional
undefined behavior. The tested x86-64 GCC and Clang configurations perform the
observable initialization eagerly.

ISO C++ permits [deferred initialization of inline variables](https://eel.is/c++draft/basic.start.dynamic).
Because this example never actually executes a use of `ghost<int>`, the observed
startup output is not promised on every conforming implementation. Keep the
tested behavior and the language's portability boundary separate.

The ingredients are established language mechanisms. Their arrangement here was
independently derived by Tor Shepherd and Codex; historical priority is not
claimed. This is a standalone instantiation trick, with no roughly-Turing claim.

## Crossing over with GCC cache memory

The ghost can instead have a constexpr initializer that calls the earlier cache
spell's `remember` function. A failed concept then leaves a **compile-time**
memory write, readable by a later `static_assert`. This combination passes the
GCC checks in the [follow-up investigation](../../.agents/notes/15-seance.md#follow-up-can-the-ghost-write-into-gccs-constexpr-cache).

There is an even smaller route: a nested requirement such as
`requires (remember(key, value, event) == value);` directly forces the write,
before a later requirement makes the concept false. It needs neither the ghost
variable nor auto return-type deduction. This is a useful application of the
existing cache storage, rather than a new independent spell. Repeated queries
and repeated writer identities can replay old results instead of writing again.

## Controls and compiler evidence

The [separate controls](experiments/seance-controls.cpp) cover rejected overloads,
`sizeof`, `decltype`, accepted requirements, ordinary false branches, discarded
dependent `if constexpr` branches, direct unevaluated variable references, and
function-local static variables. An `abort()` at the start of each auto helper
confirms that its body is never called.

The standalone source prints exactly one `boo`; the controls check six unique
initializations and their exact ID set. Both pass local GCC 13.3 at C++20
`-O0`/`-O2` and remote x86-64 GCC 16.2 and Clang 22.1.0 at C++20 `-O2`, with
`-Wall -Wextra -pedantic-errors`. These runs rely on the eager initialization
described above. Run the controls from the repository root:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors tricks/seance/experiments/seance-controls.cpp -o /tmp/seance-controls
/tmp/seance-controls
```

The cache crossover sources live with the [GCC memoization family](../gcc-memoization/).
They preserve the depth, cache, and repeated-call identity limitations, including
a GCC version/optimization difference in optional return-expression folding.
The [research notes](../../.agents/notes/15-seance.md) give the exact expectations.

## Try it

Recorded compiler evidence: **x86-64 GCC 13.3, GCC 16.2, and Clang 22.1.0; eager dynamic initialization**, C++20.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors tricks/seance/seance.cpp -o /tmp/seance
/tmp/seance
```

Expected output: exactly one `boo`. Compiling alone does not check the ghost.
