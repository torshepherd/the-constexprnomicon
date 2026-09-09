# Seance — rejected checks leave runtime ghosts

September 9, 2026. Tor requested another standalone trick, explicitly separate
from the just-completed empty-base graph-coloring work. The target remained a
surprising language mechanism, not an ordinary constexpr algorithm or a larger
application of an existing spell.

## Result

[Seance](../../tricks/seance/seance.cpp), with an [approachable explainer](../../tricks/seance/README.md), makes a
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

[seance-controls.cpp](../../tricks/seance/experiments/seance-controls.cpp) records specialization IDs in a
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
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors tricks/seance/seance.cpp -o /tmp/seance
/tmp/seance
g++ -std=c++20 -O2 -Wall -Wextra -pedantic-errors tricks/seance/experiments/seance-controls.cpp -o /tmp/seance-controls
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

## Follow-up: can the ghost write into GCC's constexpr cache?

Tor asked whether concept-driven materialization could trigger the earlier
memoization trick, and whether that combination adds any new magic. **It works;
the classification is an application of existing storage, not another primitive.**

[seance-cache.cpp](../../tricks/gcc-memoization/experiments/seance-cache.cpp) provides bounded controls. Change the ghost
from a printing runtime initializer to a required constant initializer:

```cpp
template<int Key>
inline constexpr int ghost = slow(Key, 500);

template<int Key>
auto summon() { return ghost<Key>; }
```

The false concept still triggers helper-body instantiation. That now requires a
constexpr variable's definition and its constant initialization. Unlike the
non-constant original ghost, reading this constexpr integer need not odr-use it;
the relevant bridge is its required constant evaluation. `slow(Key, 500)` warms
the old compiler cache. A later `__builtin_constant_p(slow(Key, 520))` detects it.
There is no runtime initializer or eager-startup dependency in this variation.

The source checks a cold key, checks a false concept, then successfully observes
the warmed key. Putting the failed type requirement first or giving the helper
an explicit return type leaves other keys cold on the tested configurations.

### The more revealing reduction

A nested requirement can perform the same constant evaluation directly:

```cpp
template<class T, int Key>
concept direct = requires {
    requires (slow(Key, 500) == Key);
    typename T::missing;
};
```

For `int`, the first requirement warms the cache, the second fails, and the
concept is false. Seance's variable and auto-return helper are unnecessary here.
[Nested requirements](https://eel.is/c++draft/expr.prim.req.nested) require a
satisfied constraint expression; a
[simple requirement](https://eel.is/c++draft/expr.prim.req.simple) such as
`slow(Key, 500);` merely checks an unevaluated expression. Even naming a consteval
function in that simple requirement does not itself require executing its body.
Those two simple-requirement controls leave their keys cold.

[seance-cache-memory.cpp](../../tricks/gcc-memoization/experiments/seance-cache-memory.cpp) copies the three existing
dictionary primitives unchanged and demonstrates complete writes:

1. Key 77 initially reads zero.
2. A false concept's materialized constexpr ghost writes 42.
3. Another false concept's direct nested requirement overwrites it with 99.
4. Repeating the first query leaves 99 in place.
5. A fresh query type with the old write identity also leaves 99 in place:
   the identical `remember(77, 42, 1000)` tuple can replay its cached result.
6. A fresh query and fresh write identity write 42 again.

The constexpr ghost's ordinary value is not the dictionary. Later reads recover
the payload from cached recursion facts, exactly as in the original memory spell.
Only the place initiating the write has changed. This could supply a compile-time
log of reached, instrumented checks, including failed ones. It supplies neither
new storage, automatic recurrence, rollback, nor an isolated concepts machine.

Keep the distinction between the query result and its footprint: all negative
test concepts remain false. Do not turn changing compiler memory into a promise
that one concept specialization can alternate between true and false. The
[atomic-constraint rules](https://eel.is/c++draft/temp.constr.atomic) require
consistent satisfaction for identical atomic constraints and template arguments.
Repeated queries also need not revisit an existing template definition; fresh
template identity alone does not defeat the constexpr function's separate cache.

### An optimization-dependent wrinkle

The control `auto body_only() { return slow(Key, 500); }` has no constexpr local
or other required constant-expression context. Its body is instantiated for type
deduction. Nevertheless, GCC sometimes folds that ordinary return expression
and leaves cache state. GCC 13.3 leaves its key cold at O0 and warm at O2; GCC
16.2 warms it at both O0 and O2. This was discovered by a failed cold-key
assertion, then isolated with explicit expectations. Do not equate "not required
to constant-evaluate" with "cannot be constant-evaluated by the implementation."
The forced-constexpr and direct nested-requirement constructions do not need
this optional folding behavior.

### Follow-up compiler evidence

All checks use `-std=c++23 -Wall -Wextra -pedantic-errors`; local runs also use
`-fsyntax-only`. The bounded source defaults to cold=0, warmed=1, folded=1.

| Source/configuration | Result |
| --- | --- |
| Bounded controls, GCC 13.3 O0, `-DEXPECT_FOLDED=0` | Pass; optional return-expression folding leaves key cold |
| Bounded controls, GCC 13.3 O2 | Pass |
| Bounded controls, GCC 16.2 O0 and O2 | Pass with default expectations |
| Bounded controls, GCC 13.3 O2, `-fconstexpr-cache-depth=0 -DEXPECT_WARM=0 -DEXPECT_FOLDED=0` | Pass; warmed keys are no longer observable |
| Bounded controls, GCC 13.3 O2, `-fconstexpr-depth=1024 -DEXPECT_COLD=1` | Pass; cold keys become observable too |
| Bounded controls, Clang 22.1.0 O2, `-DEXPECT_WARM=0 -DEXPECT_FOLDED=0` | Pass; no GCC-style persistent warm-key effect |
| Full dictionary crossover, GCC 13.3 O0/O2 and GCC 16.2 O2 | Pass; reads 0, 42, 99, 99, 99, 42 |

The first GCC 16.2 O0 run incorrectly expected the GCC 13.3 O0 folding result;
only its folded-key assertion failed. The corrected expectation passes. This
compiler difference does not affect the two required-evaluation constructions.
All successful checks above have no diagnostics. Remote compiler IDs were
rediscovered as `g162` and `clang2210`; compiler response codes were checked.

The [documented GCC cache and depth options](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html)
describe implementation controls, not a supported stateful-storage facility.
The hybrid inherits the original nonportable cache behavior and wrapper-identity
traps. Run raised-depth experiments only on the bounded controls, not the
dictionary's unbounded revision scan.

Reproduce the default GCC O2 checks from the repository root:

```sh
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/gcc-memoization/experiments/seance-cache.cpp
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/gcc-memoization/experiments/seance-cache-memory.cpp
```
