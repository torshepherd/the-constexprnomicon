# Last rites — September 8, 2026

## Direction and outcome

Tor asked for a brand new cursed technique. The initial exploration was drifting
toward template-parameter-object interning, and he redirected it:

> Hmm I think we’re rabbitholing a bit into the latest secret copy technique… can you look for something totally out of left field?

That direction was abandoned before any interning code was tested. The selected
[last-rites.cpp](../last-rites.cpp) instead uses **temporary destruction order as
a reverse dependency traversal**. Its application is numerical reverse-mode
automatic differentiation. This session did not resume the pointer, cache,
hidden-copy, or secret-equality experiments.

The core occupies 28 lines, followed by eight compile-time checks. It has no
includes, templates, explicit graph traversal, dynamic allocation, or compiler
builtins. GCC accepts the selected source; Clang constant evaluation has an
unresolved disagreement. No bug report was filed and nothing remains running.

## Mechanism

`number` contains the forward value, a mutable gradient initially zero, two
operand pointers, and their local derivatives. The multiplication operator
records the opposite operand's value as each local derivative; addition records
one for both. Operands are taken by const reference, so both named inputs and
temporary subexpressions can be used. `mutable` permits gradient updates through
those references. The input objects used in the assertions are created within
the enclosing constant evaluation.

`backward()` is rvalue-qualified. It sets the root gradient to one and returns
the forward value; it does not visit the graph. On cleanup, each destructor
adds `gradient * local_derivative` to its two operands. Each operation result
was constructed after its operands, so its destructor runs before theirs.
The operands receive all downstream contributions before propagating their own.
Repeated leaves, as in `x*x`, correctly receive two additions.

For `x*x + x*y` at `(3,4)`, the forward value is 21. Addition sends one to both
multiplication nodes. The square contributes 6 to x; the product contributes
4 to x and 3 to y. Final gradients are therefore `(10,3)`.

The relevant rules are [C++23 class.temporary paragraphs 4, 6.9, and 8](https://timsong-cpp.github.io/cppwp/n4950/class.temporary):
ordinary temporaries survive the full expression, reference parameters preserve
their argument temporaries until that boundary, and destruction reverses
construction. A parent result follows both operands regardless of the order in
which sibling operands were evaluated. Floating-point addition order is still
not promised to be identical across compilers.

The language's cleanup machinery supplies the scheduling, not zero-cost storage.
Each temporary stores its own operands and local derivatives; storage grows with
the expression. This is also ordinary runtime C++ in the tested GCC shape.

## Verification of the selected source

SHA-256:

```text
a328639c5f9a1b6958bd6bf61d55a9a4ab60f9761a7fe10a47b514f8bf0fb8e9
```

[Editable Godbolt, GCC and Clang side by side](https://godbolt.org/z/jaoEYe4Y4).
Compiler IDs were discovered from the live `/api/compilers/c++` endpoint.

| Compiler | Flags | Result |
| --- | --- | --- |
| Local Ubuntu GCC 13.3.0 (`13.3.0-6ubuntu2~24.04`) | `-std=c++23 -O0 -Wall -Wextra -pedantic-errors -c` | All eight assertions pass; no diagnostics |
| Same GCC | `-std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -fno-elide-constructors -c` | All eight assertions pass; no diagnostics |
| Godbolt `g162`, x86-64 GCC 16.2 | `-std=c++23 -O2 -Wall -Wextra -pedantic-errors` | Compiler code 0; no diagnostics |
| Godbolt `clang2210`, x86-64 Clang 22.1.0 | Same ordinary O2 flags | Compiler code 1; nested temporary lifetime diagnostics |
| Same Clang | Ordinary O2 flags plus `-fexperimental-new-constant-interpreter` | Compiler code 1; five false static assertions |

Disabling optional copy elision does not disable C++17's guaranteed prvalue
construction. Copy construction and copy assignment are deleted in the source;
the returned prvalues construct their result objects directly.

The assertions cover the forward value and two-variable gradient; a cubic;
branching nested expressions; literal operands; zero and negative inputs;
accumulation across separate expressions; an unseeded graph; and cleanup timing.
They use exactly representable small numbers rather than numerical tolerances.

The timing control is significant:

```cpp
number x{3};
bool before_cleanup = ((x*x).backward(), x.gradient == 0);
// Cleanup happens after the comma expression, before this next statement.
return before_cleanup && x.gradient == 6;
```

An ordinary runtime harness reads x from a command-line argument, evaluates
`x*x + x*y` with y=4, and asserts `x.gradient == 2*x_value+4` and
`y.gradient == x_value`. GCC 13.3 at `-O1 -g
-fsanitize=address,undefined -fno-omit-frame-pointer` passed with inputs 3 and
-2. LeakSanitizer cannot inspect the process in this container, so runtime
checks use `ASAN_OPTIONS=detect_leaks=0`; address and undefined-behavior
instrumentation remain enabled. This tests those calls, not arbitrary graphs.

## Clang reduction

Removing all differentiation leaves this complete control:

```cpp
struct N {
    mutable int value = 0;
    const N* p;
    constexpr N(const N* p = nullptr) : p(p) {}
    N(const N&) = delete;
    constexpr ~N() { if (p) p->value += 1; }
};
constexpr N link(const N& x) { return {&x}; }
static_assert([] {
    N x;
    link(link(x));
    return x.value == 1;
}());
```

At C++23 O2 with warning/pedantic flags, GCC 16.2 accepts it; Clang 19.1.0
(`clang1910`) and Clang 22.1.0 reject the outer destructor's write into the
inner temporary with “assignment to object outside its lifetime is not allowed
in a constant expression”. The temporary ordering rules above are the reason
this is treated as an unresolved compiler disagreement, not evidence that the
source explicitly destroys an operand early.

Two separate reduced controls also fail on Clang 22.1.0: remove `mutable` and
write through `const_cast<N*>(p)`, or store `int*` pointing directly to the
mutable gradient instead of `const N*`. Neither is a selected workaround.
No compiler-source diagnosis was completed. The experimental evaluator's false
assertions do not by themselves establish its exact destruction order or cause.

## Lifetime and semantic boundaries

- Keep every operation node as a temporary in one full expression; local input
  variables must outlive that expression. Seeding and observing gradients belong
  on opposite sides of the cleanup boundary.
- Do not retain a nested operation result in a named variable. Its inner
  temporary operands will die at the initializer's semicolon; the surviving
  object retains dangling pointers. Deleted copying does not prevent direct
  initialization of such an object.
- A deliberately invalid control with `auto y = x*x*x;` followed by
  `static_cast<number&&>(y).backward();` and `return true;` was nevertheless
  accepted by local GCC 13.3 at C++23 O0 with pedantic flags. **Do not claim GCC
  reliably diagnoses graph escape.** Standard lifetime reasoning still rules
  this shape out. This invalid control was not executed at runtime.
- Graphs are consumed during cleanup, so there is no replay. Input gradients
  accumulate across successive expressions; reset them explicitly for separate
  answers. General named intermediate graphs, assignments, graph returns from
  helpers, and lifetime extension are outside this small spell's contract.
- The implementation supports addition and multiplication. Use finite values,
  intermediate results, local derivatives, and gradients. In particular, zero
  gradient times a nonfinite local derivative is not a reliable way to discard
  an unseeded contribution.

## Provenance and computational-power scope

Reverse-mode automatic differentiation is established mathematics and software.
The [Stan Math paper](https://arxiv.org/abs/1509.07164), especially sections 3–5,
describes operator-overloaded nodes and a reverse stack traversal. A search also
found Stan's [2020 RAII discussion](https://discourse.mc-stan.org/t/proposal-raii-for-nested-autodiff/12861),
which uses destruction for nested memory recovery, not this expression-cleanup
backpropagation mechanism. These are related work, not claims of exact matches.

This temporary-destructor construction was independently assembled in this
session by Codex in collaboration with Tor. Searches around differentiation,
destructors, temporary destruction, and RAII did not establish historical
novelty; several results were too broad to be useful. Say new to this repository,
not first-ever autodiff via destructors.

Under [Pedantics](Pedantics.md), this is **not a demonstrated isolated
destructor-only roughly-Turing machine**. Cleanup supplies a finite reverse
schedule; fields supply memory; ordinary arithmetic and `if` statements perform
updates and guard missing operands. There is no claim that destruction itself
supplies all storage, selection, and an unbounded programmable loop.

A potential later question is whether the same cleanup schedule is useful for
another reverse dataflow calculation. No extension, compiler bug report, or
further search is pending automatically.
