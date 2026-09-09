# False idols

[Source](false-idols.cpp) · [Proof and verification](../../.agents/notes/12-false-idols.md)

Overload resolution decides satisfiability of a Boolean formula even when every
atomic constraint is literally `true`. There are no function bodies or recursive
solving helpers in the C++ source.

Each positive and negative literal gets its own named concept. `X` and `NX` both
evaluate to true; `NX` represents a negative literal and is not C++ `!X`. A second
formula describes inconsistent selections:

```cpp
template<class T> concept contradiction =
    (X<T> && NX<T>) || (Y<T> && NY<T>);
```

The original SAT problem has no solution exactly when its encoded formula
implies this contradiction formula. C++ preserves named atomic identities and
compares AND/OR structure during constraint subsumption. For these monotone
formulas, its ordering test realizes that implication query.
The [standard's subsumption rule](https://eel.is/c++draft/temp.constr.order)
and [atomic identity rules](https://eel.is/c++draft/temp.constr.atomic) are the
ingredients; the reduction and proof are in the research notes.

```cpp
template<class T> struct oracle {
    void solve() requires contradiction<T>;
    void solve() requires (formula<T> && true);
};
template<class T> concept satisfiable =
    !requires(oracle<T> o) { o.solve(); };
```

Both overloads are viable. For an unsatisfiable problem the second is more
constrained; for a satisfiable one they are incomparable and the call is
ambiguous. The final `true` contributes a fresh atomic identity, preventing
reverse subsumption and equal-formula ties. The `requires` expression observes
the final answer without calling either function.

The standalone file includes UNSAT and SAT cases. It passes GCC 13.3, GCC 16.2,
and Clang 22.1.0 in C++20. The newer compilers also pass C++23 at `-O2` and C++20
at `-O0` with constexpr depth limited to one. An independent generated suite
checks 389 cases on all three compilers. Hiding the formula behind an ordinary
Boolean preserves its value but destroys the structural UNSAT proof.

Inputs are source-level constraint expressions. The spell does not extract a
satisfying assignment, and finite SAT solving is not an isolated roughly-Turing
machine. Normalization can grow exponentially; this is not a practical SAT
solver or a performance claim. Corentin Jabot discusses that existing cost in
[his Clang implementation account](https://cor3ntin.github.io/posts/clang21/#faster-subsumption).
The all-true encoding and ambiguity oracle were independently assembled in this
project; the targeted prior-art search does not establish historical priority.

## Try it

Recorded compiler evidence: **GCC 13.3, GCC 16.2, and Clang 22.1.0**, C++20.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors -c tricks/false-idols/false-idols.cpp -o /tmp/false-idols.o
```
