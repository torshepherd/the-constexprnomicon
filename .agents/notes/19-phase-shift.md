# Phase shift follow-up

September 10–11, 2026. Tor asked to dig further into the structured-binding idea
from [note 18](18-out-of-left-field-search.md). This is an incremental research
record; only explicitly reported compiler results count as evidence.

## Scope and questions

- Reproduce the exact example already published in note 18 across compilers.
- Test ordinary functions, references to one shared object, and arity controls.
- Test whether a `tuple_size` specialization can decompose its subject while
  computing its own base-class argument, before becoming complete.
- Keep point-of-instantiation consistency separate from a mutable-state claim.
  [temp.point](https://eel.is/c++draft/temp.point) makes inconsistent meanings
  for a single specialization at different instantiation points ill-formed, no
  diagnostic required. Ordinary, separately defined functions avoid that trap.

## Starting state

The local commit `7f5f42b` and published `53252b3` have identical trees. The local
commit is preserved on `research-backup-7f5f42b`; the clean checkout was aligned
to published `main` before new edits. Earlier `/tmp` probes are gone, so the
published example was extracted directly from note 18 into a new scratch path.

The notes already preserve the rejected initializedness direction. Nothing in
this continuation reclassifies it as an unrelated new mechanism.

## Initial results

- The exact final source block in note 18 passed public Compiler Explorer on
  GCC 16.2 (`g162`) and Clang 22.1.0 (`clang2210`) with
  `-std=c++17 -O2 -Wall -Wextra -pedantic-errors`. Both responses reported code
  zero and no diagnostics. Its SHA-256 is
  `73cdefa7cf0feba67c7fbbf5e5072769e3ae029cec067fc6f60308ab4000e2fa`.
  This was the already-published source, following Tor's explicit publication
  authorization. No new unpublished follow-up probe was uploaded.
- The bootstrap probe passed local GCC 13.3 with C++20, O0, warnings, pedantic
  errors, and `-fsyntax-only`. A lambda in the base-class argument of
  `tuple_size<coordinate>` decomposes `coordinate{1, 2}` into two members and
  returns their sum, three. Once the specialization is complete, later bindings
  use three synthetic elements. This does not discover a generic field count:
  the initializer's ordinary arithmetic supplies the value three.
- A const-wrapper caching hypothesis failed. An early `const auto& [x, y]`
  did not leave later const bindings with two elements while mutable bindings
  saw three: GCC rejected the later two-name binding because the type decomposes
  into three elements. Removing the early function produced the same rejection.
- An unused template parameter did not freeze a nondependent two-member binding
  before completion. Instantiating that function afterward made GCC diagnose
  the mismatch against three elements. The draft's nondependent hypothetical-
  instantiation rule also blocks using changing interpretations as a portable
  trick; see [temp.res.general](https://eel.is/c++draft/temp.res.general).

The latter probes used local GCC 13.3, C++20, O0, warnings, pedantic errors, and
`-fsyntax-only`. They are rejected experiments, not working storage primitives.
The sources are now preserved in
[the experiment directory](../../tricks/phase-shift/experiments/).

## Reference, template, and rejection controls

The continuation completed a 14-case local matrix on Ubuntu GCC
13.3.0-6ubuntu2~24.04. All cases used `-Wall -Wextra -pedantic-errors
-fsyntax-only`. Eight positive compilations passed without diagnostics:

- The [unchanged main spell](../../tricks/phase-shift/phase-shift.cpp),
  [reference control](../../tricks/phase-shift/experiments/references.cpp), and
  [late generic lambda](../../tricks/phase-shift/experiments/late-template.cpp)
  each passed C++17 at O0 and O2.
- [Bootstrap](../../tricks/phase-shift/experiments/bootstrap.cpp) passed C++20
  at O0 and O2.

Six negative compilations rejected for the expected reasons:

| Source / option | Mode | Intended primary diagnostic |
| --- | --- | --- |
| References, `-DEARLY_THREE` | C++17/O0 | Three names; type decomposes into two elements |
| References, `-DLATE_TWO` | C++17/O0 | Two names; type decomposes into three elements |
| References, `-DOMIT_GET` | C++17/O0 | `get` not declared; no fallback to members |
| Rejected const probe | C++20/O0 | Later const binding still requires three names |
| Rejected const probe, `-DOMIT_EARLY` | C++20/O0 | Same later arity rejection |
| Rejected nondependent template | C++20/O0 | Instantiation requires three names, not two |

The reference control passes one object through both ordinary functions.
Address comparisons confirm aliases to the same real members, not merely two
different temporary copies. Earlier binding increments `x` by one; later binding
increments `x` by two and `y` by three. `{10, 20}` ends as `{13, 23}`. The computed
third element is a snapshot of the pre-update sum, not extra state inside the
object or a reactive formula.

The dependent generic lambda is deliberately defined before completion but only
called after it. It sees three elements. This positive control is separate from
the rejected file, so the failed nondependent-template assertion does not hide
its verdict. No claim that one specialization changes meaning is needed.

An argc-dependent runtime driver also passed with zero exit status both with
no extra argument and with one extra argument, using GCC 13.3/C++17/O0 with the
same warnings. It is retained as
[runtime.cpp](../../tricks/phase-shift/experiments/runtime.cpp). Reproduce with:

```sh
g++ -std=c++17 -O0 -Wall -Wextra -pedantic-errors tricks/phase-shift/experiments/runtime.cpp -o /tmp/phase-shift-runtime
/tmp/phase-shift-runtime
/tmp/phase-shift-runtime extra
```

The runtime control is important: the switch is in declaration visibility, not
constant-evaluator bookkeeping. No constexpr-only claim survives this control.

## Standards reading and assessment

- [dcl.struct.bind](https://eel.is/c++draft/dcl.struct.bind): completeness plus
  `value` chooses the tuple protocol; missing accessors do not undo that choice.
- [class.mem.general](https://eel.is/c++draft/class.mem.general): the bootstrap
  lambda appears in a base-specifier, outside the member-specification's
  complete-class contexts. The specialization is still incomplete there.
- [tuple.helper](https://eel.is/c++draft/tuple.helper): the size trait is an
  integral-constant unary trait; cv wrappers have substitution-dependent rules.
  The rejected const probe is evidence against that specific cache hypothesis,
  not a full explanation of a library implementation's instantiation strategy.
- [temp.res.general](https://eel.is/c++draft/temp.res.general) and
  [temp.point](https://eel.is/c++draft/temp.point): differences between
  hypothetical and actual interpretation, or among a specialization's
  instantiation points, can be ill-formed without a required diagnostic.

The standalone result is real and mechanistically separate from Empty bits and
GCC memoization. Its limits are equally real: a one-time source-level protocol
switch, with normal accessors and ordinary arithmetic doing the value work.
The bootstrap is a sharper presentation, not a new storage primitive. We did
not find a zero-size-integer-caliber capability or an isolated machine here.

The main source, detailed walkthrough, and every concrete follow-up probe are
retained under [Phase shift](../../tricks/phase-shift/). Main-source remote
responses were re-inspected after the interruption: both code zero, empty
stderr, and the checked source hash still matched. Follow-up probes were not
sent remotely; do not silently extend the Clang claim to them.

## Future leads, not a pending work queue

- C++26 structured-binding packs might let a bootstrap inspect native members
  with less fixed-arity boilerplate. A generic adapter would need a careful
  instantiation-consistency audit; ordinary member counting alone is not a new
  capability. Not tested.
- A bootstrap returning reference views could make an amusing self-defining
  adapter, but it would still need ordinary accessor machinery. Not tested as
  a generic construction; the concrete reference control above is the extent
  of current evidence.
- Do not chase repeated trait mutation or cross-translation-unit disagreement
  as if this example established either. Do not resume the rejected const-cache
  direction without a substantively different, standards-audited premise.

## Documentation audit

The earlier note's `typeid` code block had lost the glvalue from the successful
probe: `T{}` is not a polymorphic glvalue. Restore the local object and reference
cast, and distinguish it from the uncompiled function-returning-reference idea.
The old trinary entry also incorrectly excluded the recorded source-specific
GCC 16.2/Clang 22.1 remote verdicts as a stale request; the session record showed
the request rebuilt from the corrected bit-field source. Correcting that record
does not reverse Tor's same-mechanism rejection. These are documentation
corrections, not discoveries made by this phase-shift follow-up.

The corrected `typeid` block and retained trinary block were each extracted from
note 18 and recompiled locally at C++23/O0 with GCC 13.3, warnings, pedantic
errors, and `-fsyntax-only`: both passed without diagnostics. These are new
local audit checks, not reruns of the historical remote matrix.
