# Seance — rejected checks leave runtime ghosts

September 9, 2026. Tor requested another standalone trick, explicitly separate
from the just-completed empty-base graph-coloring work. The target remained a
surprising language mechanism, not an ordinary constexpr algorithm or a larger
application of an existing spell.

## Result

[Seance](../seance.cpp), with an [approachable explainer](../seance.md), makes a
false concept bring a printing global-variable specialization into the program.
The executable has an empty `main` and prints `boo` once on the checked targets.
Changing the helper's return type from `auto` to `int` makes it silent while
preserving the concept's false value and the successful assertions.

This works through return-type deduction, odr-use, and dynamic initialization.
There is no constexpr evaluation, mutable evaluator metadata, subsumption
solver, empty object storage, or friend injection. Runtime storage and runtime
printing are real; the surprising part is which compile-time query causes the
runtime initializer to become part of the program.

## Derivation and boundary

A template helper `auto summon() { return ghost<T>; }` must have its body
instantiated to determine its return type, even for a call used only in a
requires-expression or `decltype`. Its variable read odr-uses the corresponding
non-constant global variable specialization. That specialization has a dynamic
initializer. A later failed requirement does not roll back this instantiation.

The call is unevaluated; the instantiated body is a separate context. Do not say
that requires evaluates its operand or that printing happens during compilation.
The printing happens when the compiled executable starts on the checked targets.

Inline-variable dynamic initialization may instead be deferred by a conforming
implementation. Here no executed use of the individual inline ghost forces it
to happen. Therefore the startup output is an observed implementation behavior,
not an unconditional ISO portability claim. There is no intentional undefined
behavior in the source. Relevant primary language references:

- [Placeholder return-type deduction](https://eel.is/c++draft/dcl.spec.auto):
  instantiation can be needed to find a return type; the standard includes an
  unevaluated `decltype` example.
- [Odr-use](https://eel.is/c++draft/basic.def.odr): the variable read is in a
  potentially evaluated expression in the instantiated function body.
- [Implicit instantiation](https://eel.is/c++draft/temp.inst): distinguishes
  needed definitions from declarations that can remain uninstantiated.
- [Requires-expressions](https://eel.is/c++draft/expr.prim.req.general): lexical
  checking order, false results on substitution failure, and stopping early.
- [Dynamic initialization](https://eel.is/c++draft/basic.start.dynamic):
  unordered initialization of instantiated specializations and permitted
  deferral for inline variables.

## Controls

[seance-controls.cpp](seance-controls.cpp) records specialization IDs in a
zero-initialized unsigned mask and separately counts initializations. It expects
IDs 0, 2, 4, 5, 8, and 9, exactly once each. This observes a set, not startup order.
It prints `seance controls: six ghosts, zero calls` and returns zero on success.

| Probe | Expected footprint |
| --- | --- |
| Query helper, then fail a nested-type requirement | One ghost |
| Two distinct concept queries reaching the same ghost specialization | Still one ghost |
| Fail nested-type requirement before reaching helper | None |
| Reject a constrained overload in an unevaluated call | One ghost; undefined fallback selected |
| Give helper explicit `int` return type | None |
| Use auto-returning helper in `sizeof` | One ghost |
| Use auto-returning helper in `decltype` | One ghost |
| Direct `sizeof` of an explicitly typed variable template | None |
| Replace namespace variable with function-local static | None without an actual call |
| Satisfying nested-type requirement, positive concept control | One ghost |
| Instantiate an ordinary `if (false)` branch with a dependent query | One ghost |
| Dependent query in a discarded `if constexpr` branch | None |

Every `summon` body in the controls begins with `std::abort()`. Successful
execution therefore also verifies that these bodies are never called. The
overload pair is only declared; successful linking confirms that resolving the
unevaluated call does not require executing either candidate.

The ordinary-if control's query was made dependent on `T` so that its footprint
is attributable to instantiating the containing specialization, rather than
checking a nondependent query while initially parsing the template.

Early probes used `record<T>()` with `__PRETTY_FUNCTION__` to show type names.
They logged `int` from a rejected overload and `double` from a direct false
concept query before a main-entry message, on all three compilers. The root
spell drops the builtin and labels to expose the essential mechanism.

## Compiler evidence

All final builds use C++20, `-Wall -Wextra -pedantic-errors`. These are build-and-
execute checks, not syntax-only checks. All listed final runs had successful
build and process exit codes and no compiler diagnostics.

| Source | Local GCC 13.3.0 | CE GCC 16.2, `g162` | CE Clang 22.1.0, `clang2210` |
| --- | --- | --- | --- |
| `seance.cpp` | O0, O2: exactly `boo` | O2: exactly `boo` | O2: exactly `boo` |
| `seance-controls.cpp` | O0, O2: exact mask and count | O2: exact mask and count | O2: exact mask and count |
| Root with `auto summon()` changed to `int summon()` | O2: empty stdout | Covered by explicit-return control | Covered by explicit-return control |

Local compiler identifies itself as `g++ (Ubuntu 13.3.0-6ubuntu2~24.04) 13.3.0`.
Remote IDs were discovered through Compiler Explorer's compiler-list API.
Execution requests checked `buildResult.code`, top-level `didExecute` and `code`,
and stdout. No permanent Godbolt share link was requested; the previous session's
known publication refusal did not need to be repeated.

Reproduce from the repository root:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors seance.cpp -o /tmp/seance
/tmp/seance
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors working-notes/seance-controls.cpp -o /tmp/seance-controls
/tmp/seance-controls
```

The minimal explainer version is the root spell with the independent `warded`
definition and the two extra assertions removed. No runtime use of `ghost<int>`
should be added to the positive test: that would independently instantiate or
initialize the very variable whose indirect appearance we are observing.

## Provenance and further threads

Independent derivation in this Tor Shepherd/Codex session; not a claim of first
historical discovery. Auto-return body instantiation, template-based static
registration, and instantiation side effects are existing mechanisms. The
[2008 SFINAE expression proposal](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2008/n2634.html)
already explicitly discusses instantiation side effects outside the immediate
context. That is background for the mechanism, not an assertion that the paper
contains this runtime ghost construction.

Targeted searches combined unevaluated/requires/decltype/return-type deduction
with dynamic initialization, registration, and rejected candidates. They found
general registration and instantiation discussions, but no verified exact match
for the false-concept/auto-to-int demonstration. Limited search cannot establish
historical novelty.

Useful possible applications, not unfinished deliverables:

- A runtime census of reached instrumented template specializations could use
  the same trigger. It would record presence, not how many times a query ran.
- Rejected-overload labels could help inspect which instrumented constraints
  were reached. Do not promise a complete trace or the compiler's visitation
  order: early stopping, reuse, and unordered startup initialization matter.
- A non-inline variable plus an ordinary same-translation-unit use might tighten
  the deferred-initialization argument. That variant was not tested or claimed;
  keep the honest implementation boundary of the small checked spell.

Do not grow a registry framework automatically. No roughly-Turing claim is made.
The discovery search also considered exception-specification inference, special-
member generation, pointer qualification composition, and lookup-based graph
queries, but did not compile constructions for those routes. They are ideas,
not tested failures or additional discoveries.
