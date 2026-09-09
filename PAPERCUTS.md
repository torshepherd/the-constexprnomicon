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
