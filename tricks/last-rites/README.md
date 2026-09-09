# Last rites

[Source](last-rites.cpp) · [Editable Godbolt](https://godbolt.org/z/jaoEYe4Y4) ·
[Research handoff](../../.agents/notes/11-last-rites.md)

The semicolon differentiates the expression:

```cpp
static_assert([] {
    number x{3}, y{4};
    double result = (x*x + x*y).backward();
    return result == 21 && x.gradient == 10 && y.gradient == 3;
}());
```

Each arithmetic operator returns a temporary containing its value, pointers to
its operands, and two local derivatives. `backward()` only sets the root's
gradient to one and returns its value. The destructor adds its gradient times
each local derivative to the corresponding operand's gradient.

Operands are constructed before the result that depends on them. At the end of
the full expression, temporary destruction supplies the reverse traversal:
each result propagates before its temporary operands are destroyed. The
[C++23 temporary rules](https://timsong-cpp.github.io/cppwp/n4950/class.temporary#8)
provide that ordering. Sibling evaluation order can vary; either order respects
these dependencies. As usual, floating-point accumulation can depend on order.

There is no separately allocated tape, explicit traversal, or recursive
backward call. The temporary objects still occupy storage proportional to the
expression. This is numerical reverse-mode differentiation using ordinary
arithmetic and `constexpr` destructors; it does not produce a symbolic formula.
No templates or compiler builtins appear in the spell.

Keep operation nodes as temporaries within one full expression, with input
variables surviving it. Read gradients in the next statement. Saving a nested
expression in a named variable can leave it pointing at already-destroyed
temporaries. Copies and assignments are deleted, but direct initialization can
still create that invalid lifetime arrangement. Each expression's graph is
consumed once; input gradients accumulate across successive expressions.
The small spell implements `+` and `*` and assumes finite arithmetic throughout.

All eight assertions pass on GCC 13.3 at `-O0` and `-O2`, and on Godbolt GCC
16.2 at `-O2`, with C++23 and pedantic diagnostics. The GCC 13.3 O2 check also
disables constexpr caching and optional copy elision. Clang 22.1.0's default
evaluator rejects the nested cases with a lifetime diagnostic; its experimental
new evaluator also fails assertions. The reduced disagreement and runtime
sanitizer check are recorded in the handoff. No Clang portability claim is made.

Reverse-mode differentiation and operator-overloading AD are established work;
the [Stan Math paper](https://arxiv.org/abs/1509.07164) describes an explicit
reverse traversal of recorded nodes. This session independently assembled the
temporary-destructor scheduling variant. The limited prior-art search did not
establish historical novelty.

## Try it

Recorded compiler evidence: **GCC 13.3 and 16.2 (Clang rejects nested cases)**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -c tricks/last-rites/last-rites.cpp -o /tmp/last-rites.o
```
