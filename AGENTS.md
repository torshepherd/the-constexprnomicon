# Working in the Constexprnomicon

This is a collection of small, artistic compile-time C++ experiments.

For session history and research handoff, start at
[working-notes/README.md](working-notes/README.md). It links the decisions,
successful and unsuccessful probes, compiler evidence, and shelved work.
Keep research logs there; keep the root README for readers of the spells.
Read [PAPERCUTS.md](PAPERCUTS.md) for known workflow friction and workarounds.

Read [working-notes/Pedantics.md](working-notes/Pedantics.md) before assessing a
technique's computational power. It preserves Tor's verbatim instructions on
roughly-Turing, isolation, allowed operations, and intermediate versus final
value extraction.

- Commit and push completed work directly to `main`; do not open pull requests
  for routine work in this repository. Tor explicitly prefers this workflow:
  nobody depends on the repository, and it is a collaborative experiment.
  Preserve remote history and incorporate concurrent changes without force-pushing.
- Reduce each trick to its essence. Favor concise, deliberate expressions when
  they clarify the trick; complexity should come from the idea.
- Keep each spell standalone. Put the code before its compile-time checks.
  Add an approachable Markdown explainer beside the source, with the same stem:
  `spell.cpp` and `spell.md`. Build the explanation from first principles, with
  small code steps and diagrams or tables where useful; do not assume a math/CS
  course background. Link the pair from the README. Use this convention for new
  spells and add missing explainers when revisiting existing ones. NOTES.md keeps
  the technical overview, limitations, and provenance; working-notes keeps the
  research history and detailed compiler evidence.
- Write down and commit every observed papercut in PAPERCUTS.md, including small
  inconveniences that burn cycles: refusals or approval surprises, commands that
  fail, missing tools, confusing behavior, retries, and workarounds. Do not limit
  the log to major or unresolved blockers. Record the context, attempted action,
  actual symptom or stated rejection reason, extra work caused, workaround, and
  current status. Update existing entries when they recur or are resolved, and
  commit the log alongside the session's work.
- Preserve the playful style. Avoid turning sketches into a generic library or
  adding infrastructure without a concrete need.
- Keep meaningful static_assert checks for the claimed behavior. Verify changed
  spells on the named compiler and record the version, flags, and outcome.
- Treat compiler acceptance as evidence of observed behavior, not proof of ISO
  portability. Document the assumptions a reader needs to reproduce the spell.
- Credit related work and the human/AI collaboration. Distinguish independent
  derivation from a verified claim of historical novelty.
- This repository has its own history. Reference sorcery-cpp as the pre-AI
  original; do not import or rewrite that repository's history.
