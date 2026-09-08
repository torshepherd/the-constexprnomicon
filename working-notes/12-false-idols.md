# False idols — SAT in all-true constraints

Research from the September 8, 2026 session, independently assembled by Tor
Shepherd and Codex. Tor accepted the finding and requested publication of the
spell, README, working documents, and research leads. The standalone source is
now part of the root spell collection.

**Claim:** C++20 constraint subsumption can decide satisfiability of an encoded
Boolean formula, even though every atomic constraint evaluates to `true`.
The C++ solving code has no function definitions, assignment search, recursive
templates, constexpr functions, friend injection, or compiler builtins.

The [standalone example](../false-idols.cpp) includes a satisfiable control.
[Additional controls](subsumption-controls.cpp) check the structural information
channel and the guard. The [regression generator](false-idols-check.py) checks
389 instances against an independent ordinary brute-force solver.

## Tor's target — verbatim

> Ah to be clear it doesn’t have to be constexpr C++.
>
> Maybe you want to expand a little and read about other techniques I invented, like CTAD-only programming: https://github.com/torshepherd/sorcery-cpp/blob/master/talks/static_assert_false/examples/ctad/1.cpp
>
> The world is much bigger than just “evaluate in constexpr with compiler loopholes”

> I think there are 2 interesting angles
>
> - tricks that expose really weird things to say out loud, like “zero size integer”. wtf haha
> - individual language features like ctad where it is surprising that you can achieve all of the ingredients of writing roughly-Turing complete programs exclusively with that feature, like ctad-only.

He explicitly requested a new direction, not another development of empty bits,
compiler caching, copy gates/secret equality, or destructor-driven gradients.
The CTAD source was read; this session did not attempt to repair its internal
`.value` extractions.

## The two meanings of true

Declare a distinct named concept for each positive and negative literal:

```cpp
template<class> concept X  = true;
template<class> concept NX = true;
```

`NX` is NOT C++ `!X`. These are independent symbolic atoms. Their mathematical
relationship is supplied separately by the encoding below. Their runtime/constant
Boolean values are both true, so they never exclude a candidate from viability.

But subsumption preserves an atom's source identity and parameter mapping.
Distinct named definitions provide distinct atoms even when their values agree.
Thus a compiler can compare the structure of formulas whose ordinary values are
identical. Relevant rules: [atomic identity](https://eel.is/c++draft/temp.constr.atomic)
and [partial ordering by constraints](https://eel.is/c++draft/temp.constr.order).

## The reduction

Write a CNF formula using positive symbols `Xi` for positive literals and distinct
symbols `NXi` for negative literals. Let its resulting monotone formula be `F`.
Define a second monotone formula:

```text
Bad = (X1 AND NX1) OR (X2 AND NX2) OR ...
```

As a symbolic formula, `Bad` means at least one variable has been assigned both
polarities. Then:

**The original formula is unsatisfiable iff F implies Bad.**

- A satisfying assignment selects true positive symbols or true negative symbols
  consistently. It makes `F` true and `Bad` false, disproving the implication.
- Conversely, a symbolic assignment making `F` true and `Bad` false contains no
  contradictory pair. Extend any unassigned variables arbitrarily. Because `F`
  is monotone in the literal symbols, this gives a satisfying ordinary assignment.

It is not necessary to require every variable to have a chosen polarity. The
first probe did so; those domain clauses were subsequently removed.

For formulas consisting only of AND/OR over distinct atoms, C++'s DNF-to-CNF
subsumption test is exactly monotone implication: every DNF term on the left
must intersect every CNF clause on the right. If a term and clause have no common
atom, setting that term's atoms true and that clause's atoms false supplies a
counterexample; a shared atom prevents one. This is a derivation from the
standard rule, not a claim that C++ understands general logical negation.

The complete observation apparatus is:

```cpp
template<class T> struct oracle {
    void solve() requires Bad<T>;
    void solve() requires (F<T> && true);
};
template<class T> concept satisfiable =
    !requires(oracle<T> o) { o.solve(); };
```

The final `true` is a fresh atomic constraint, NOT a simplifiable logical identity
for subsumption. It makes reverse subsumption impossible. Therefore:

| Original formula | Ordering | Observation |
| --- | --- | --- |
| UNSAT | `F && fresh` strictly subsumes `Bad` | Unique viable best overload; `requires` is true |
| SAT | Neither constraint subsumes the other | Ambiguous call; `requires` is false |

The guard also handles `F` structurally equivalent to `Bad`. Omitting it can leave
an UNSAT case tied. Both functions are non-template members of a class template;
the early free-function-template prototype was replaced to avoid depending on
the functional-equivalence rules for function templates. Compare the standard's
[constrained-member example](https://eel.is/c++draft/temp.over.link#example-4).

## Example and controls

The four clauses in the standalone example rule out all four assignments of two
variables. Removing the final clause admits `x = y = true`. Both C++ concept
expressions still evaluate to true; the overload result changes.

The separate controls show that hiding `F<T>` in an ordinary Boolean variable
template erases its internal constraint structure. Its value stays true but the
UNSAT proof disappears. They also check that the guard resolves equal formulas.

An attempted compact indexed atom, `template<class, int I> concept atom = (I == I)`,
passed the two GCC probes but failed the satisfiable control on Clang 22.1.0.
No compiler bug was diagnosed. It was discarded; the retained construction uses
separate named, literally-true definitions and passes both compilers.

## Verification

All reported runs completed successfully, with no diagnostics unless stated.

| Check | GCC 13.3 local | GCC 16.2 / Godbolt | Clang 22.1.0 / Godbolt |
| --- | --- | --- | --- |
| Standalone, C++20, `-Wall -Wextra -pedantic-errors` | Pass | Pass at `-O2` | Pass at `-O2` |
| Standalone, C++20, `-O0 -fconstexpr-depth=1 -ftemplate-depth=8` | Pass, with controls and ops limit below | Pass | Pass |
| Standalone, C++23, `-O2 -Wall -Wextra -pedantic-errors` | Not separately run | Pass | Pass |
| Generated suite, C++20, warnings/pedantic | Pass | Pass at `-O2` | Pass at `-O2` |
| Additional structural and guard controls, C++20, `-O0` | Pass | Not separately run | Not separately run |

The local additional-control command also set `-fconstexpr-depth=1`,
`-fconstexpr-ops-limit=16`, and `-ftemplate-depth=8`.

The 389-case suite contains:

- Every subset of eight elementary non-tautological clauses over two variables:
  four unit clauses and four two-variable clauses (256 cases).
- 64 seeded random formulas for each of three and four variables (128 cases).
- Four repetition/tautology/contradiction controls and one equal-formula guard
  control (five cases).

There are 202 SAT and 187 UNSAT expectations. The Python generator enumerates
assignments only to produce independent expected answers. The generated C++
contains no search implementation or answers hidden in its constraints; answers
appear only in final `static_assert` checks.

From the repository root:

```sh
g++ -std=c++20 -Wall -Wextra -pedantic-errors -fsyntax-only false-idols.cpp
g++ -std=c++20 -O0 -fconstexpr-depth=1 -fconstexpr-ops-limit=16 -ftemplate-depth=8 -fsyntax-only working-notes/subsumption-controls.cpp
python3 working-notes/false-idols-check.py
```

The generator can print source with `--source` or send it to a discovered
Godbolt compiler ID as its argument. It uses the documented compile endpoint;
HTTP failure is not a compile result. Verify IDs before future remote use.

## Limits, isolation, and novelty

This meets the first aesthetic target: **a SAT solver made of overload ambiguity,
with every atomic constraint true and no function bodies.** It is not an isolated
roughly-Turing demonstration. There is finite symbolic search, but no demonstrated
unbounded recurrence or evolving memory within this restricted mechanism.

The allowed machinery is named concepts, AND/OR constraint composition, constraint
subsumption to order two overloads, and a final `requires` expression plus Boolean
negation to observe the answer. No intermediate answer is extracted and fed to a
different evaluator. Template parameters are scaffolding, not assignment states.

Inputs are source-level constraint expressions, not arbitrary runtime data or
strings. There is no satisfying-assignment extraction yet. Compiler limits can
prevent large cases from compiling; an incomplete compilation is not a SAT/UNSAT
answer. The reduction does not imply efficient SAT solving.

Subsumption and its worst-case normalization explosion are existing language and
implementation facts. Corentin Jabot discusses both in
[his Clang implementation account](https://cor3ntin.github.io/posts/clang21/#faster-subsumption).
The distinctive construction here is the positive/negative all-true literal
encoding, forbidden-pair implication query, fresh-atom guard, and ambiguity oracle.
Targeted web searches found no exact prior example. That is not a historical
priority proof. Do not call it the first SAT solver through C++ constraints.

Tor accepted the core finding. The [next-direction handoff](13-research-directions.md)
records specific experiments worth trying, their isolation boundaries, and
directions considered but not tested. Publishing these leads does not mean any
of them has been demonstrated. Avoid automatically expanding the small spell
into a SAT library or large application framework.
