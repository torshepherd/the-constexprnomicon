# Working in the Constexprnomicon

A collection of small, artistic C++ experiments, often at compile time.

## Start here

- [Research handoff](.agents/notes/README.md): decisions, compiler evidence,
  unsuccessful probes, shelved work, and next directions. Read the relevant
  topical notes before continuing an investigation.
- [Pedantics](docs/Pedantics.md): Tor's verbatim rules for computational-power
  claims. Read before assessing isolation or roughly-Turing completeness.
- [Papercuts](.agents/PAPERCUTS.md): known workflow friction and workarounds.

## Repository layout

- `README.md` is the short reader-facing catalog; link to each trick's folder.
- `tricks/<name>/<name>.cpp` is the small standalone spell, with code before its
  meaningful `static_assert` checks. `tricks/<name>/README.md` explains it from
  first principles, with compiler versions, flags, limits, and provenance.
- Keep larger applications in named subdirectories of the parent trick, with
  their own source and README. Keep controls and exploratory code in that
  trick's `experiments/`, linked and explained by its README or research notes.
- Group a shared mechanism before adding catalog entries: GCC cache storage
  lives under `tricks/gcc-memoization/`, with counter, memory, vector, sortable
  vector, and cache crossover experiments together. Its family README links
  each construction. Keep the root introduction to the one-line tagline.
- `docs/` holds material shared across tricks: machine investigations,
  Pedantics, and common provenance. Keep standalone tricks and isolated-machine
  claims distinct; update the machine page when evidence actually changes.
- `.agents/notes/` holds research history and handoffs. Preserve user quotations
  verbatim. Keep its README a navigation aid; put session detail in topical notes.
- `.agents/` is this repository's convention for supporting agent material.
  Root `AGENTS.md` remains the discoverable instruction entry point. These notes
  are not skills or automatically loaded configuration.

## Working rules

- Commit and push completed work directly to `main`; do not open routine PRs.
  Tor explicitly prefers this workflow because nobody depends on the repository.
  Incorporate concurrent changes and preserve remote history; never force-push.
- Reduce each trick to its essence. Preserve the playful style and favor concise,
  deliberate expressions. Keep every spell standalone; avoid generic-library
  machinery or infrastructure without a concrete need.
- Build explainers in small code steps; do not assume a math/CS course background.
  Keep the mechanism, caveats, and reproduction instructions together. Maintain
  links and commands when moving files; commands should run from the repo root.
- Verify changed spells on their named compiler and record version, flags, and
  outcome. Compiler acceptance is evidence, not proof of ISO portability.
  For pure moves, verify source identity and relocated includes/commands.
- Write down and commit **every observed papercut**, including minor friction
  and successful workarounds: refusals or approval surprises, failed commands,
  missing tools, confusing behavior, retries, and extra steps. Record context,
  attempted action, symptom or stated rejection reason, extra work, workaround,
  and status in `.agents/PAPERCUTS.md`. Update recurring entries; include the log
  in the session's commit. Do not include credentials.
- Credit related work and the human/AI collaboration. Independent derivation
  does not establish historical novelty. This repo has its own history:
  reference sorcery-cpp as the pre-AI original; do not import or rewrite its history.
