# Papercuts

Small things that cost collaborators time: refusals, confusing tool behavior,
missing capabilities, failed approaches, extra steps, and workarounds. Tor
requested this log on September 9, 2026. Record minor friction too, including
problems that were eventually worked around. Commit the entries with the work.

For each entry, record when and where it happened, the attempted action and
observed symptom, the time or extra work it caused, the workaround if any, and
what remains unresolved. Do not invent timing estimates or diagnoses. Update
existing entries with new occurrences and resolutions instead of repeating the
same discovery. Keep credentials and other sensitive data out of the log.

## 2026-09-09 — Public Godbolt share link rejected after compilation succeeded

- **Context:** Chromatic aberration research in ChatGPT Work. Public Compiler
  Explorer API requests had successfully compiled the source on GCC and Clang.
- **Symptom:** Automatic approval review rejected creating a permanent Godbolt
  shortlink. Its stated reason was that research/testing authorization did not
  authorize permanent publication or disclosure of the source. The rejection
  surfaced when polling the running shell command.
- **Cost:** A failed share-link attempt, extra tool handling, and an explanation
  to the user, despite successful compiler checks and an established workflow
  for publishing completed work to this repository.
- **Outcome:** No permanent link was created. The checked source and reproduction
  commands shipped in the separately authorized repository commit. Compilation
  access worked; creating a share link remained blocked.

## 2026-09-09 — Git fetch works, but Git push has no credentials

- **Context:** The same Work workspace could clone/fetch this public repository
  over HTTPS. A local commit succeeded.
- **Symptom:** `git push origin HEAD:main` failed with
  `fatal: could not read Username for 'https://github.com': No such device or address`.
- **Cost:** Publishing required switching interfaces and reconciling the local
  commit with a commit created through the connected GitHub app.
- **Workaround:** Read the current remote `main`; use the app's Git tree/commit
  tools to publish the changed files on that parent; update the branch with
  `force: false`. Fetch the result and verify its tree equals the local tree.
  In this session the clean, agent-owned local commit was then reconciled with
  `git reset --soft origin/main`. Matching trees and a clean working tree were
  checked first; this was not a reset of unrelated work.
- **Status:** The app completed the push. Shell push credentials were not fixed.
  The app-created commit has a different SHA because its commit metadata differs,
  even though its file tree is identical.

## 2026-09-09 — Truncated command output breaks JSON handoff

- **Context:** Preparing several complete Markdown files and sources as JSON for
  the GitHub tree API, including existing contents of the longer `NOTES.md`.
- **Symptom:** A 17,000-token command-output limit truncated the roughly
  18,141-token result. `JSON.parse` failed on the tool's leading warning instead
  of receiving the intended JSON object.
- **Cost:** Another file read and an output-format investigation during publishing.
- **Workaround:** A 22,000-token limit returned the complete payload. The
  orchestration code retained it internally and printed only a compact summary.
  Prefer bounded per-file reads or smaller batches for future handoffs; don't
  assume command output is complete JSON after truncation.
- **Status:** Worked around. Full existing file contents can be much larger than
  the diff being published.
- **Seance recurrence:** Combining several long working notes into one command
  also truncated a read, obscuring part of the research-directions document.
  Re-read the missing section with a bounded `sed` range. Large discovery output
  is a problem even before publishing; keep subsequent reads focused.

- **Cleanup recurrence:** Printing several independent document reads in one
  orchestration result hit the outer output budget even when each individual
  command fit its own limit. Re-read the needed portions with bounded ranges.

## 2026-09-09 — Only an older GCC is installed locally

- **Context:** This Work workspace provided Ubuntu GCC 13.3.0 as `g++`.
- **Symptom:** `clang++ --version` reported `command not found`. The compiler
  names in the repository's example commands are not necessarily installed here.
- **Cost:** Cross-compiler verification required remote requests and tracking
  local versus remote evidence separately.
- **Workaround:** Use the installed GCC for quick probes. Discover available
  Compiler Explorer IDs and use its API for GCC 16.2 and Clang 22.1.0. Both remote
  targets worked in this session.
- **Status:** Local Clang was not installed. Check availability once when entering
  a new environment; these are observations about this workspace, not a permanent
  limitation of Work or evidence that Clang rejects the spell.
- **Seance recurrence:** The fresh workspace again had GCC 13.3 and no `clang++`.
  A version check confirmed the missing executable. Remote GCC 16.2 and Clang
  22.1.0 compiled and executed both new sources successfully; no local compiler
  installation was needed.

- **Cleanup recurrence:** A broad local compile sweep included the one-pointer
  vector, whose documented GCC version is 16.2. GCC 13.3 rejected local-variable
  constant expressions. The source matches the original byte for byte, so this
  did not indicate a move regression. Recorded the unsupported configuration on
  its README and kept the existing newer-compiler evidence. Match each example's
  advertised compiler before assuming the available local compiler is suitable.

## 2026-09-09 — Multiple old checkouts make session startup ambiguous

- **Context:** Four earlier scratch workspaces contained checkouts of this
  repository at different commits; some also had uncommitted changes.
- **Symptom:** A broad file search found several `AGENTS.md` files and working-note
  sets. An older checkout's instructions were read before the latest clean
  checkout was identified.
- **Cost:** Comparing commit tips and working-tree states before research could
  proceed; a stale checkout could also lead to repeating completed work.
- **Workaround:** Inspect each candidate's tip and status, preserve its local
  changes, and create an isolated checkout for the current session from the
  newest clean candidate. Fetch remote `main` before relying on that history.
- **Status:** The current session has its own checkout. Existing working
  directories were left intact; scratch paths should not be treated as durable
  references to the latest project state.

## 2026-09-09 — A wrong-context hunk rejects an entire multi-file edit

- **Context:** This documentation follow-up, while creating the explainer and
  papercut log and updating their index links.
- **Symptom:** The agent put a hunk from another document into the
  `working-notes/README.md` patch. `apply_patch` rejected it because the expected
  text was absent. No part of that multi-file patch was applied.
- **Cost:** An extra status/read check and resubmission of the edits. This was an
  agent editing mistake, not a repository or compiler failure.
- **Workaround:** Re-read the actual target context, then separate file creation
  from the smaller updates to existing documents.
- **Status:** The incorrect patch was discarded; the corrected edits use the
  target files' current text.

## 2026-09-09 — Assumed source directory interrupts startup inspection

- **Context:** Seance research, reading the newly cloned repository.
- **Symptom:** A batched `rg --files working-notes spells` failed because spells
  live at the repository root; there is no `spells` directory. A dependent
  `git log` after `&&` consequently did not run.
- **Cost:** An extra bounded file read and later history check. This was an agent
  assumption, not a repository problem.
- **Workaround:** Use the root README's paths and inspect the known working-notes
  directory. Keep independent inspection commands independent of a speculative
  path succeeding.
- **Status:** Resolved; no repository restructuring was needed.

## 2026-09-09 — Broad tool discovery produces mostly irrelevant output

- **Context:** Seance startup, looking for repository and context capabilities.
- **Symptom:** Filtering tool metadata by generic words such as `file` and
  `search` printed many unrelated capabilities. Later full remote execution
  payloads similarly included build-environment metadata beyond the needed result.
- **Cost:** Extra output to inspect before reaching the useful tool signatures
  and compiler outcomes.
- **Workaround:** Filter by exact capability names and keep full response objects
  internally. Print build status, execution status, diagnostics, and stdout.
- **Status:** Agent workflow improvement; no capability was missing or blocked.

- **Cleanup recurrence:** An initial broad `search|git|terminal|exec` metadata
  filter again returned irrelevant tools. Narrow discovery to the exact GitHub
  capabilities once the checkout exists.

- **Astral heap recurrence:** Startup again printed a broad tool inventory
  before discovering that ordinary Git worked after sandbox escalation. A later
  combined working-notes/web read exceeded the outer output budget. Re-read the
  relevant notes in smaller ranges and kept subsequent compiler output bounded.

## 2026-09-09 — Web reader fails on a supplied GitHub source link

- **Context:** Reading Tor's earlier CTAD example during the Seance search.
- **Symptom:** The web reader returned `Internal Error` for the supplied GitHub
  blob URL, while the Compiler Explorer API documentation opened successfully.
- **Cost:** Needed a separate raw-source HTTP fetch; that fetch continued beyond
  the initial shell wait and required retrieving the completed output.
- **Workaround:** Fetch the same file from GitHub's raw-content URL. The complete
  source was returned successfully.
- **Status:** Resolved; not evidence that the source was inaccessible or missing.

- **Astral heap recurrence:** The web reader returned `Internal Error` for two
  GCC GitHub source views. The exact release sources had already downloaded
  successfully from `raw.githubusercontent.com`; local bounded reads verified
  the implementation and line numbers. No source-access blocker remained.

## 2026-09-09 — Desktop sandbox blocks network cloning and WSL discovery

- **Context:** Astral heap research started in a Windows ChatGPT project mirror
  with only its instructions and read-only `sources/`, not a repository checkout.
- **Symptom:** The sandboxed HTTPS clone failed to connect through the configured
  loopback proxy. Windows command discovery found no C++ compiler. Sandboxed
  `wsl --list --quiet` returned `Wsl/EnumerateDistros/Service/E_ACCESSDENIED`.
- **Cost:** An extra clone attempt, compiler-location inspection, and WSL retry
  before experiments could start.
- **Workaround:** Approved escalated commands cloned the public repository into
  a new child directory and identified Ubuntu with GCC 13.3 and Clang 18.1.
  Compiler runs then used that distribution with explicit workspace paths.
- **Status:** Resolved through the supported escalation path. There was no
  automatic-review rejection and no edit to synced project source files.

## 2026-09-09 — Nested Windows/WSL shell handling loses loop variables

- **Context:** Running a small compiler/exponent matrix during Astral heap work.
- **Symptom:** A Bash loop passed through PowerShell and `wsl ... bash -lc`
  reached its Python child without the compiler-name argument or exponent value.
  The child tried to execute `-std=c++23` and raised `FileNotFoundError` eight
  times. The precise shell layer consuming the variables was not established.
- **Cost:** One failed matrix invocation and replacing its orchestration.
- **Workaround:** Put the loop and argument lists in a workspace Python file;
  invoke it directly with `wsl --cd <workspace> -- python3 <file>`. No shell
  variable interpolation is involved in compiler arguments.
- **Status:** Resolved. Later compilation and expected-rejection results were
  taken from the actual compiler return codes, not wrapper success alone.

## 2026-09-09 — Equivalent-looking giant initializations take different paths

- **Context:** Trying to zero-initialize a huge constexpr array after sparse,
  uninitialized allocation had worked in GCC 13.3.
- **Symptom:** `new unsigned char[1ull << 60]{}` exceeded the default constexpr
  loop limit. Simply having few later reads/writes did not make this form cheap.
- **Cost:** A failed bounded probe and investigation of aggregate initialization.
- **Workaround:** A local aggregate-initialized built-in array worked; putting
  the array in `struct realm` and allocating with `new realm{}` retained the
  sparse representation. Source inspection confirmed indexed initializer entries
  and implicit default values.
- **Status:** Explained and preserved as a spell limitation, not a request to
  raise limits or perform a huge runtime allocation.

## 2026-09-09 — Windows and WSL Git disagree about checkout line endings

- **Context:** A final Astral heap audit invoked Linux Git on the checkout
  created by Windows Git, alongside compilation through WSL.
- **Symptom:** Linux `git diff --check` treated existing CRLF files as changed
  and reported trailing whitespace throughout the repository, producing a large
  truncated output. The audit stopped before its dependent object-file check.
  Windows Git showed only the intended edits and warned about normal LF-to-CRLF
  checkout conversion for newly edited files.
- **Cost:** One failed audit, an output flood, and a separate working-tree check.
- **Workaround:** Keep Git inspection and publication in Windows Git, which
  created this checkout; use WSL only for compiler and filesystem-based checks.
  Remove the cross-environment Git command from the audit and rerun the remaining
  object check. Do not normalize the whole repository to silence this mismatch.
- **Status:** Resolved without changes to unrelated files or global Git settings.

## 2026-09-09 — Fresh desktop checkout has no Git author identity

- **Context:** Saving the verified Astral heap spell using the repository's
  standing commit-to-main workflow.
- **Symptom:** Windows Git staged and checked the intended files, then refused
  `git commit` with `Author identity unknown` and an auto-detection failure.
  No commit was created. The desktop has a credential helper but no configured
  `user.name` or `user.email`.
- **Cost:** A failed commit and inspection of the existing commit metadata.
- **Workaround:** Use the connected GitHub app's tree/commit/ref tools, as in the
  earlier publishing workaround, allowing the authenticated service to supply
  commit identity. Do not invent an author email or alter global Git settings.
- **Status:** Publication uses the connected app; local author configuration
  remains unchanged. A final fetch and matching-tree check reconcile the checkout.

## 2026-09-09 — Temporary probe directory disappears across a pause

- **Context:** Resuming Seance after the user's rate-limit continuation message.
- **Symptom:** Writing an extracted documentation example to the earlier `/tmp`
  probe directory failed with `FileNotFoundError`. The repository and its pending
  source/documentation edits were still present, but that temporary directory was
  gone. The cause of its removal was not established.
- **Cost:** One failed verification command and recreating a probe directory.
- **Workaround:** Put the reproducible final check in a fresh workspace scratch
  directory. Earlier compiler outcomes were already recorded in working notes;
  the source, controls, and handoff did not need reconstruction.
- **Status:** Resolved. Never depend on temporary request/result files as the only
  record of an expensive investigation.

## 2026-09-09 — Optional constant folding changes a cache control's expectation

- **Context:** Seance/cache crossover research; checking an uncalled auto-return
  helper containing an ordinary constexpr function call.
- **Symptom:** An initially expected cold-cache assertion passed on GCC 13.3 O0
  but failed on GCC 13.3 O2: return-expression folding had warmed the cache.
  Reusing the O0 expectation on GCC 16.2 also failed, because that version warmed
  the key even at O0.
- **Cost:** Needed to separate required constant evaluation from optional folding
  and rerun the bounded control with the observed per-compiler expectation.
- **Workaround:** Use an explicit constexpr initializer or a nested requirement
  for the principal construction. Preserve the optional-folding probe with a
  separate expectation parameter and exact compiler/optimization evidence.
- **Status:** Explained and recorded in working-notes/15-seance.md. This was a
  research assumption corrected by the controls, not a failed cache write or
  evidence that the unevaluated call was executed.

## 2026-09-09 — Compiler command matcher misses C++ executable names

- **Context:** Updating reproduction commands while moving sources into trick folders.
- **Symptom:** A migration regex used a word boundary immediately after `g++` or
  `clang++`. The final `+` and following space are both non-word characters, so
  command lines were skipped and still pointed at old source locations.
- **Cost:** A source-path inspection exposed stale commands and required a second
  targeted migration before verification. No compiler failure was needed to find it.
- **Workaround:** Match executable names followed by whitespace, including version
  suffixes, and replace complete source-path tokens. Audit every documented
  compiler/Python command against the resulting tree.
- **Status:** Fixed during cleanup. Historical scratch filenames and `/tmp` output
  paths are deliberately preserved; repository input paths are updated.
