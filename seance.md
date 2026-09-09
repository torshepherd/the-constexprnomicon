# Seance

[Source](seance.cpp) · [Technical notes](NOTES.md#seance) · [Research and controls](working-notes/15-seance.md)

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

The root source checks `haunted<int>` twice and `warded<double>` once. It prints
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
