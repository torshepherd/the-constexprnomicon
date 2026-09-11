# Fine print

**A compile-time parser whose entire program lives in function declarations.
Every function returns `void`. None has a body. The answer is whether it is
`noexcept`.**

[The spell](fine-print.cpp) checks balanced parentheses. The same mechanism
also runs a [generic cyclic tag machine](cyclic-tag/): its queue can grow, its
instructions repeat, and its next transition depends on its data.

[Editable Godbolt: both examples, GCC and Clang](https://godbolt.org/z/e78xTGzWP).

## 1. A function can have an answer without having a body

```cpp
void yes() noexcept;
void no() noexcept(false);

static_assert(noexcept(yes()), "true");
static_assert(!noexcept(no()), "false");
```

These are declarations. The calls inside `noexcept` are unevaluated; no
definitions are required. The operator inspects exception specifications and
produces a compile-time Boolean. It does not inspect a missing function body
or experimentally try to throw an exception.
See [the noexcept operator](https://eel.is/c++draft/expr.unary.noexcept).

## 2. An exception specification can refer to the next one

This is an ordinary conditional exception specification:

```cpp
void first() noexcept(noexcept(yes()));
```

The outer `noexcept(...)` declares `first`'s property; the inner one queries
`yes`'s property. In a function template, that inner query can instead select
another overload with different template arguments. Determining its exception
specification can initiate the next query, and so on.

The useful distinction is timing: dependent exception specifications are
instantiated separately, when needed. Selection of a function by overload
resolution is one such need. Unselected candidates can keep their specifications
uninstantiated. This is language machinery, independently of GCC's constexpr
memoization. See [exception specifications](https://eel.is/c++draft/except.spec#13)
and [implicit instantiation](https://eel.is/c++draft/temp.inst#16).

## 3. Turn the declarations into a parser

```cpp
template<char...> struct word {};

void balanced(...) noexcept(false);
void balanced(word<>, word<>) noexcept;

template<char... Input, char... Stack>
void balanced(word<'(', Input...>, word<Stack...>)
    noexcept(noexcept(balanced(word<Input...>{}, word<'(', Stack...>{})));

template<char... Input, char... Stack>
void balanced(word<')', Input...>, word<'(', Stack...>)
    noexcept(noexcept(balanced(word<Input...>{}, word<Stack...>{})));
```

The first argument is unread input. The second is the stack of unmatched opens.
The empty `word` template carries characters in its type, with no members or
operations of its own.

| Selected declaration | Next configuration |
| --- | --- |
| Input begins with `(` | Remove it from input; push an open onto the stack. |
| Input begins with `)` and the stack has an open | Remove both characters. |
| Both input and stack are empty | Stop with `noexcept(true)`. |
| Anything else | Select the ellipsis fallback; stop with `noexcept(false)`. |

For `(())`, the successive configurations are:

| Unread input | Stack |
| --- | --- |
| `(())` | empty |
| `())` | `(` |
| `))` | `((` |
| `)` | `(` |
| empty | empty |

The final true specification propagates back through all four transitions.
For `)(`, the pop overload cannot match an empty stack, so the fallback yields
false immediately. For `(()`, input runs out while the stack is nonempty, also
selecting the fallback.

```cpp
static_assert(noexcept(balanced(word<'(','(',')',')'>{}, word<>{})), "balanced");
static_assert(!noexcept(balanced(word<')','('>{}, word<>{})), "unbalanced");
```

The apparent function calls describe the next configuration; none executes.
There is no `constexpr` function, `if`, arithmetic, class-template partial
specialization, member-value extraction, or compiler builtin in the parser.
The explicit return type is always `void`.

## What counts as the mechanism

Under this repository's [Pedantics](../../docs/Pedantics.md), the allowed
combination is **function-template deduction and overload selection, pack
substitution, and lazy exception-specification propagation**.

| Ingredient | Where it happens |
| --- | --- |
| Selection | Matching the leading character and the stack shape selects an overload. |
| Memory and updates | Packs carry state; deduction separates heads from tails and substitution rebuilds the next state. |
| Recurrence | A selected exception specification asks for the next configuration's specification. |
| Observation | The outermost `noexcept` yields the terminal Boolean. |

No intermediate `.value`, helper evaluator, or function body takes over. The
balanced parser is a pushdown computation; it alone does not establish general
universality. The [cyclic tag application](cyclic-tag/) supplies a general
machine encoding within the same combination. This is an explicitly combined
mechanism, not a claim that the `noexcept` operator alone, or overload resolution
alone, supplies every ingredient.

## Reproduce

From the repository root:

```sh
g++ -std=c++11 -O0 -Wall -Wextra -pedantic-errors -fconstexpr-depth=1 -fsyntax-only tricks/fine-print/fine-print.cpp
g++ -std=c++11 -O0 -Wall -Wextra -pedantic-errors -fconstexpr-depth=1 -ftemplate-depth=128 -fsyntax-only tricks/fine-print/experiments/controls.cpp
python3 tricks/fine-print/experiments/check.py
```

| Verification | GCC 13.3 local | GCC 16.2, CE `g162` | Clang 22.1.0, CE `clang2210` |
| --- | --- | --- | --- |
| Parser, cyclic machine, positive controls, earlier two-tag sketch | C++11/O0; C++23/O2 | C++11/O2 | C++11/O2 |
| 511 parentheses cases and 940 terminating cyclic-machine cases | C++11/O0 | C++11/O2 | C++11/O2 |

All passed with warnings and pedantic errors enabled and constexpr call depth
limited to 1. The local C++23 runs also disabled constexpr caching. Remote checks
combined the sources in separate namespaces and used template depth 128.
The [research note](../../.agents/notes/20-fine-print.md) records source hashes
and the deliberately rejected controls.

The Python checker uses an ordinary depth counter for parentheses and a deque
plus numeric instruction pointer for cyclic tags. Of its 1,215 cyclic cases,
792 return true and 148 false; 205 repeat a configuration and 70 exceed its
80-step bound. Those 275 unresolved cases are excluded from Boolean assertions.
They are never counted as false answers.

## Limits and provenance

This is a compile-time program. Calling the declared functions in evaluated
code would require definitions; it would not execute this parser at runtime.
Dependent calls rely on argument-dependent lookup to find the transition
overloads in the same namespace as the state types. Keep the complete overload
set before the first query. See [dependent candidates](https://eel.is/c++draft/temp.dep.candidate).

For the cyclic machine, an exact cycle can produce a self-dependent exception
specification, and indefinite growth can exceed instantiation limits. Those are
compilation failures, not negative answers. An ill-formed selected exception
specification is also a hard error, not a SFINAE fallback. The parser's false
case works because it explicitly declares a valid potentially-throwing fallback.

Conditional noexcept, template-based rewriting, and lazy instantiation are
established C++ facilities. This construction was independently assembled by
Tor Shepherd and Codex; targeted searches did not locate an exact parser or
cyclic-tag implementation using this arrangement. That is not a priority claim.
Cook's cyclic-tag model is credited in the [application](cyclic-tag/).
