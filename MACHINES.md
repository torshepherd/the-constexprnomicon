# Roughly-Turing-complete features in C++

This part of the Constexprnomicon asks whether a narrowly defined language
mechanism can support programs of its own. The [standalone tricks](README.md#standalone-tricks)
show surprising capabilities; a machine claim requires additional evidence.
One mechanism can eventually belong in both parts.

## What counts here

Tor's project-specific term **roughly-Turing** requires all three ingredients:

| Ingredient | Required demonstration |
| --- | --- |
| Selection | Choose a transition based on the current state. |
| Memory | Represent state, change it, and make the changed state available to later transitions. Immutable type representations may carry successive states. |
| Recurrence | Repeat transitions through recursion or iteration, rather than writing out every step or possible input case in advance. |

Specify the allowed operations before assessing a construction. If a helper
provides branching, state manipulation, or recurrence, include it in the named
technique. Observing the final answer is separate from extracting intermediate
answers and letting another evaluator continue the computation.

These are project criteria, not a substitute for a formal universality proof.
Compiler limits and historical-priority claims are separate questions too.
The [verbatim instructions](working-notes/Pedantics.md) are the authority for
this terminology.

## Current investigations

No complete isolated machine is currently demonstrated by the examples tracked
on this page. This is a statement about the available evidence, not a claim
that any listed feature is incapable of supporting one.

| Feature or proposed combination | Evidence in hand | Open boundary |
| --- | --- | --- |
| CTAD-only programming | Tor's [earlier merge-sort example](https://github.com/torshepherd/sorcery-cpp/blob/master/talks/static_assert_false/examples/ctad/1.cpp) expresses computation through deduction guides. | It extracts intermediate results through `.value`. A fully isolated replacement for that extraction has been proposed by Tor but not supplied and checked here. |
| Hidden copy plus overload resolution | [Copy gate](copy-gate.cpp) and the [factorial investigation](working-notes/09-copy-gate.md) demonstrate copying and deduction affecting results. | The factorial uses `if constexpr` and other helpers. It does not establish selection and recurrence within the proposed restricted combination. |
| Ambiguity-driven constrained overload resolution, observed with `requires` | [False idols](false-idols.cpp) supplies a finite SAT oracle through subsumption; `requires` observes whether its two eligible overloads have a unique winner. | Feed the result into another transition, carry evolving state, and obtain recurrence within an explicitly agreed set of operations. No such machine has yet been checked. |

## Ambiguity as a condition

A dependent `requires` expression can turn an invalid expression into a false
requirement. In particular, an ambiguous overload call can be observed this way.
It is a general validity check, not an ambiguity-specific intrinsic: missing or
deleted functions, inaccessible operations, and other failures can also make a
requirement false. False idols controls its candidates so the observed failure
is ambiguity.

There are two distinct ingredients in that spell:

- **The predicate's implementation:** constraint subsumption makes a call unique
  for UNSAT and ambiguous for SAT.
- **The observation:** a final `requires` expression, negated, exposes that answer.

Using such an answer as a condition is a promising starting point for a machine.
However, the existing spell only observes a completed query. It does not yet use
that answer to select and initiate a subsequent state transition.

The next bounded experiment is to show a state that can take either of two
transitions, perform several transitions without spelling them out, and halt.
Its explanation should identify exactly:

1. What represents the state.
2. What creates or removes the ambiguity.
3. What selects the next state.
4. What causes the next query to happen, and what terminates the process.

An ordinary recursive template or constexpr function could provide the missing
machinery. Such a construction would need to be described as a combined technique;
it would not establish that subsumption alone has those capabilities. Conversely,
if constrained overload selection can itself arrange the next query, that would
be stronger evidence worth isolating in a small standalone example.

Do not infer arbitrary-program execution from SAT solving alone. The demonstrated
oracle operates on a finite formula written in the source. No unbounded machine,
satisfying-assignment extraction, or persistent mutable concept state has been
shown. In particular, changing the satisfaction result of identical atomic
constraints with the same arguments is not a supported mutable store; the
[standard makes that ill-formed, no diagnostic required](https://eel.is/c++draft/temp.constr.atomic).

## Research handoff

[False idols: proof and controls](working-notes/12-false-idols.md) records the
verified construction. [Research directions](working-notes/13-research-directions.md)
records hypotheses and possible experiments. Update this page when an ingredient
or a complete machine is actually demonstrated, with the source, compiler evidence,
and allowed operations supporting the new status.
