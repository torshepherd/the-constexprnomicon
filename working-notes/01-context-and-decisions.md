# Context and decisions

## The aesthetic

Tor asked for compile-time C++ as art: minimal, concise, cute, clever code that
takes contemplation to understand. His instruction was:

> I don't like adding fluff to make them more complex looking; I like reducing
> to the bare essence

Postincrements inside expressions, reverse loops, and deliberate comma
expressions fit that preference. Template the useful idea on a type, but do not
grow sketches into a polished general-purpose functional library. Keep the core
spell ahead of its checks; put explanation outside the code when possible.

The request for further discoveries specifically targeted **C++23, without
reflection**. The earlier compile-time `cat` uses C++26-era facilities and is
recorded as a separate historical experiment, not advertised as a C++23 spell.

## Terminology: roughly-Turing

Tor's personal definition, supplied during the copy-gate follow-up:

> has either recursion or iteration and memory

Read this as **(recursion or iteration) + memory**. Use this definition when
assessing whether a spell is roughly-Turing. It is a practical exploration
criterion; do not silently replace it with a proof of formal Turing completeness
or an additional requirement for unbounded storage. Memory can be state carried
between successive instantiations; it need not be a mutable global object.

Name the mechanism supplying each ingredient, including any use of constexpr
evaluation, template instantiation, return-type deduction, or overload matching.
The combined program satisfying the criterion does not establish that each of
those mechanisms independently does so.

Tor also reports earlier work establishing computational power in template
argument deduction and exploring overload resolution, with a recursion obstacle
in the latter. His correction was: “TAD is but OVL is not quite”. The earlier
construction and exact restrictions have not been recovered in this session;
preserve that distinction without claiming the obstacle has been diagnosed or
solved. The [copy-gate assessment](09-copy-gate.md#roughly-turing-assessment)
applies his definition to the experiments we actually have.

## How the session developed

1. Tor asked for transcription of his
   [Static Antics talk](https://www.youtube.com/watch?v=6epTVJt1bpY) and Godbolt
   experiments with a pointer-only span. The video advertised automatic English
   captions, but caption responses were empty and the transcript endpoint
   returned HTTP 400 `FAILED_PRECONDITION`. **No transcript was produced.**
   The work used Tor's description and his repository, not a transcription of
   the whole talk.
2. A dereference probe iterated readable elements without a length. Experiments
   expanded to temporary allocations, `std::vector`, aggregate and array
   boundaries, and diagnostic-based `cat`.
3. Tor asked whether size versus capacity could support an owning one-pointer
   vector. Probing union alternatives supplied an empty/occupied distinction,
   first in a tagless number union, then in an integer container.
4. Tor requested templating and the art pass. The selected `vector<T>` probes
   only its scalar empty marker and reserves a trailing empty slot. This avoids
   depending on whether an arbitrary payload is recognized as constant.
5. Asked to go deeper, Codex developed and tested a GCC cache counter and a
   Clang empty-base bit store. Recursion headroom, evaluator failure, nested
   union state, and mutation inside probes were also explored.
6. Tor asked about provenance. A targeted search found related work, including
   explicit prior art for the active-union/lifetime probe. No exact match was
   located for the two selected compiler-state constructions, but no exhaustive
   priority claim was established.
7. Tor created this repository. He proposed **the-constexprnomicon**, selected
   the description “A grimoire of forbidden compile-time C++, conjured by human
   and machine,” and wanted it to acknowledge the collaboration.
8. Tor proposed hiding the vector's pointer in an `empty<T>` wrapper. Scalar
   packing worked on Clang; pointer packing did not. A broad pointer sweep found
   a library-name-dependent `void*` cast exception, but no way to serialize the
   pointer into the empty bit store. Tor explicitly allowed shelving the
   zero-storage vector, then requested this handoff.

There were rate-limit interruptions. “Continue” messages referred to the same
ongoing exploration; they were not requests to restart it from scratch.

## Repository identity

- [sorcery-cpp](https://github.com/torshepherd/sorcery-cpp) stays Tor's historical
  pre-AI artifact. This repository is an independent companion, **not a fork**;
  do not import or rewrite its history.
- [static_antics](https://github.com/torshepherd/static_antics/tree/4a0cdbf528e4388fd51ea6566e98f592d9598db0)
  supplied the starting point. At that snapshot the session inspected the
  README, utilities, cat, grep, Wordle, Git-branch example, and TODO process/LLM
  examples. Those use compilation and diagnostics as an I/O environment. No
  changes were made to that repository.
- The root README credits Tor Shepherd in collaboration with OpenAI's Codex.
  Preserve that credit and the restrained, playful tone.
- The first published commits were `23a5d6870d37493884658ce4386a73a38ee27a59`
  (“Open the Constexprnomicon”) and
  `8864a48f0988c9eab0831088d7ade14b5969621c`
  (“Inscribe the first three spells”). These notes cover the session after
  those commits. An unpublished local root commit with the same spell tree was
  only a preparation artifact; it is not the public repository's ancestry.

## Provenance and corrections

The constructions were independently assembled and tested in this conversation;
that describes their immediate origin, not historical priority.

[P2641R4 §3.5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2641r4.html#implementation-experience)
already describes `__builtin_constant_p` as a lifetime/active-union detection
technique. Barry Revzin and Daveed Vandevoorde's 2023 paper credits Johel Ernesto
Guerrero Peña and Ed Catmur for that approach. Early excitement about “inventing
a discriminator-free variant” must be read with this later correction.

[Filip Roséen's 2015 constexpr counter](https://refp.se/articles/constexpr-counter)
uses friend injection. It is related stateful metaprogramming, with a different
storage mechanism from the memoization cache counter here. The targeted search
did not find the exact cache counter or empty-base bit store; “not found” does
not establish that nobody previously did them.

Other distinctions future writeups should preserve:

- The builtin recognizes constants; it is not a universal pointer-validity,
  liveness, or core-constant-expression predicate.
- A language-level empty object still has `sizeof == 1` as a complete object on
  the tested target. Empty bases can avoid additional layout storage. Neither
  statement says the evaluator spends no memory on the hidden state.
- The root vector is the compact template with `push`/`pop`, not the older
  integer container with `reserve`, `clear`, and move support.
- The nested-union byte was not selected for the root collection, but its saved
  compile-time checks **passed**. Its ordinary call produced a different value;
  see [the compiler-state notes](03-compiler-state.md#the-nested-union-byte).
- No compiler bug was filed or diagnosed to completion. Documentation
  discrepancies and existing issue leads are observations, not resolved bugs.

## Where to stop

There is no working empty owning vector, and it is not the default next task.
The fake library-name cast is recorded here, not promoted to a fourth root spell.
No recheck automation was scheduled. Optional transcription services were
discussed, but none produced a transcript. Finish this handoff and let Tor choose
the next direction.
