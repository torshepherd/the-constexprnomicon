# Research directions after False idols

This is a research handoff: hypotheses, useful experiments, and decision criteria.
Only the results explicitly marked verified in
[the False idols notes](12-false-idols.md) have compiler evidence. The directions
below have not been demonstrated. They are not a claim of more discoveries.

## What to optimize for

Tor's two targets are an absurd-sounding capability and an unexpectedly complete
machine inside a narrowly defined language feature. Neither requires constexpr
evaluation. His verbatim clarification is in the preceding notes; the isolation
rules remain in [Pedantics.md](Pedantics.md).

The strongest part of False idols is the mismatch between ordinary values and
the information consumed by another compiler mechanism. All constraints are true,
yet their identities and structure still encode a question. The useful general
research pattern is to look for information the implementation must retain, then
find a language operation that observes it in an unexpected way.

Keep the core phenomenon small. A large application of a known technique is not
automatically a stronger discovery. Tor specifically disliked the progression
from striking primitives into extended follow-up investigations, and considered
the destructor-gradient example ordinary C++ computation at constexpr time.

## 1. Can subsumption return a witness?

**Question:** Can the same structural mechanism recover a satisfying assignment,
rather than only report that one exists?

A modest first experiment would use a two-variable formula with exactly one
solution. Look for a way to make overload selection expose its literal choices
without declaring an overload for every possible assignment.

The easy general approach is to ask repeated SAT queries with one variable fixed
at a time. That could make a useful application, but a helper that reads each
Boolean answer and chooses the next query adds another computational mechanism.
It must not be advertised as subsumption-only witness extraction.

Likewise, listing all assignments as overloads makes the source generator perform
the enumeration. The distinguishing goal is to have compiler-provided structure
perform the search, while source size describes the problem rather than its
whole search tree. Stop and report the boundary if the only working route is an
ordinary recursive SAT wrapper.

## 2. Can formulas be reused without erasing their structure?

**Question:** How far can a source-level formula be parameterized while remaining
transparent to constraint normalization?

Verified boundaries:

- Named concept composition retains the structure needed by the solver.
- Wrapping the formula in an ordinary Boolean variable template erases it.
- The attempted indexed atom `(I == I)` disagreed between GCC and Clang.
  The retained spell uses separate literally-true concept definitions.

An economical next experiment is to try another parameter-dependent atomic
expression with a clearly distinct mapping for each literal, preserving a pair
of SAT/UNSAT controls. Establish compiler behavior and check the standard's
equivalence rules before using it to reduce boilerplate.

Do not replace formula composition with a general constexpr predicate and assume
subsumption sees inside it. Do not assume folds or newer concept-parameter features
have the same normalization behavior as written-out C++20 AND/OR expressions.
Such extensions need their own language-version claim and controls.

## 3. Other questions encoded as implication

**Question:** What compact, recognizable problem makes the same mechanism's
behavior even clearer?

One candidate is a tiny pigeonhole instance: three pigeons in two holes. A paired
two-pigeon instance would provide the satisfiable control. This would demonstrate
a familiar impossibility using declarations alone, without building a solver
framework. No pigeonhole source was compiled in the discovery session.

Another candidate is a symbolic implication/equivalence checker for formulas
built from named atoms. The SAT reduction already uses monotone implication;
equivalence would require checking both directions. The fresh-atom guard and
observation method must be accounted for explicitly.

Treat these as applications unless they expose another surprising primitive.
Keep problem encodings small: normalization cost can grow exponentially, and a
resource-limit failure must never be reinterpreted as an UNSAT answer.

## 4. The isolated-machine question remains open

Tor subsequently identified ambiguity as a possible condition for a recursive
machine and requested that the documentation distinguish standalone tricks from
roughly-Turing-complete features. His message is preserved in
[Pedantics.md](Pedantics.md); [MACHINES.md](../MACHINES.md) is now the reader-facing
status page. The hypothesis has not yet been implemented or compiler-tested.

Subsumption supplies a rich finite decision operation. The retained construction
does not yet show evolving memory or unbounded recurrence inside that operation.

Before trying a machine, write down the allowed operations and where state lives.
In particular, identify what initiates the next step. If that step is an ordinary
function, template specialization, member-value read, or `if constexpr` consuming
the previous answer, name the combined technique rather than silently counting
the helper as subsumption.

The useful next deliverable would be a tiny repeated transition with genuine
state and a branch. Another larger SAT instance would not establish recurrence.
Finite Boolean universality and finite symbolic search are not, by themselves,
Tor's roughly-Turing criterion.

## Other seams considered, without working probes

These are alternatives for a fresh session, not failed experiments or proven
impossibilities. The discovery session surveyed their prospects, then built the
subsumption probes instead.

| Mechanism | Question worth testing | Main isolation concern |
| --- | --- | --- |
| Implicit conversion sequences | Can conversion selection propagate a computation through changing types? | Ordinary sequences allow only limited user-defined conversion chaining; explicit recursive helpers could do all the work. |
| Inferred exception specifications | Can compiler-inferred exception properties form composable logic and drive another step? | An automatically combined Boolean property is not yet a closed machine; reading it with a helper can introduce the real branch. |
| Object layout and alignment | Can layout constraints answer a nontrivial combinatorial question? | Size, padding, and alignment already combine values, but arithmetic through `sizeof` or generated classes must be counted separately. ABI choices matter. |
| Member lookup and inheritance ambiguity | Can lookup success, dominance, or ambiguity carry a reusable logical result? | A lookup gate is insufficient if SFINAE or specialization supplies all subsequent state transitions. |
| Recursive `operator->` processing | Can one arrow expression drive a changing state through compiler-provided chaining? | Return-type selection and termination may depend on ordinary template machinery; proxy chaining alone is established behavior. |
| Alias-template application | Is there a distinctive capability beyond a type-level encoding of ordinary functional computation? | A Church/SKI encoding would show computation, but does not automatically supply a novel C++ phenomenon. |

Do not describe these as compiler-rejected routes: no concrete implementation was
tested for them in this session. The important tested negative controls concern
formula opacity, the missing guard, and the indexed atom, and are saved separately.

## Starting the next session

1. Read the short [root spell](../false-idols.cpp), then the reduction in note 12.
2. Choose one bounded experiment above or an entirely new mechanism. Do not start
   by growing infrastructure or resuming the shelved pointer/cache/copy work.
3. Keep the original SAT/UNSAT pair as a control if modifying the encoding.
4. Distinguish compiler observations, a language-rule argument, and a historical
   novelty claim. The first two do not establish the third.

The user accepted False idols and explicitly authorized pushing the spell,
README, working documents, and these research directions to `main`. His subsequent
ambiguity-driven machine proposal gives the recurrence investigation a concrete
focus. The documentation split does not itself constitute a machine demonstration.
