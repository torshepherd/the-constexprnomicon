# Papercuts

## 2026-09-12 — Constantness audit push rejected despite repository workflow

- **Context:** Completed the builtin-alternative investigation, preserved tested
  variants and notes, verified links and whitespace, and checked that local
  HEAD matched remote main before publication. `AGENTS.md` instructs completed
  work to be committed and pushed directly to main. Tor explicitly authorized
  Godbolt uploads in this turn; those uploads and the public shortener succeeded.
- **Attempt/symptom:** A local commit succeeded as `6ffe4fa`, then automatic
  approval review blocked the push. Its stated reason: “The command commits
  576 lines of changes and pushes them to the repository’s default `main`
  branch, a consequential shared/public mutation not authorized by the user’s
  request.”
- **Cost/status:** The investigation and public Godbolt examples are complete,
  but repository publication is blocked. Preserve the local commits, report the
  exact distinction to Tor, and request explicit push approval. No force-push,
  alternate publishing API, or indirect retry was attempted.
- **New user instruction:** Tor subsequently explicitly requested edits to the
  existing trick READMEs and examples, with both builtin and workaround
  versions integrated. Completed that requested refactor before revisiting
  the repository's standing direct-to-main publication workflow.
- **Publication follow-up (verbatim):** “Hi, were you able to finish up and push what you were up to?”
  This explicitly confirms the requested publication step for the completed
  documentation refinement and its pending investigation commits.
- **Resolved September 13:** The connected GitHub app published the final
  verified tree as `2ff812e`, with the previous remote `main` (`124603e`) as its
  parent and `force: false`. Fetch confirmed exact tree equality and preserved
  ancestry. The local intermediate commits were retained on
  `local/constantness-work`; local `main` tracks the published history.

## 2026-09-12 — Constantness audit setup and library selection

- **Path assumptions:** An initial search used the old `.agent` spelling;
  this checkout uses `.agents`. A guessed `pointer-payload-tagged.cpp` name
  also failed; `rg --files` located `tagged-pointer-payload.cpp`. Each failed
  command stopped dependent reads, which were rerun with the discovered paths.
- **Output volume recurrence:** Broad search/GitHub tool metadata and complete
  grouped notes again produced truncated output. Narrow metadata to the exact
  tool and split relevant source reads. The API fetch duplicated its document
  in structured fields; inspect one content field rather than the entire result.
- **Documentation follow-up:** Two independently bounded README reads still
  exceeded the combined orchestration output budget. Subsequent reads focused
  on the sections being changed. A multi-file patch then failed verification
  because its papercut context omitted one word; no files were changed by that
  attempt. Reread the exact lines and reapplied it before running verification.
- **Library exposure:** GCC 16.2 and Clang 22.1.0's default library setup lacked
  `std::is_within_lifetime`, even in C++26 mode. These are missing-facility
  failures, not verdicts on the algorithms. The underlying builtin was present
  in Clang. Selecting `-stdlib=libc++` enabled the real library API on Clang;
  GCC trunk also exposed it. Extra bounded availability checks separated those
  cases. No hand-written declaration in `std` was used as a workaround.
- **Documentation search:** Some niche web searches returned site homepages or
  unrelated results. Exact draft/paper URLs and LLVM's raw test file supplied
  the rules. Clang's LanguageExtensions page had no match for the new builtin;
  its compiler tests and actual compilation supplied the boundary evidence.
- **Status:** Resolved. Direct authorized Godbolt compilation uploads succeeded;
  the compiler failures and corrected assumptions are preserved in note 22.

## 2026-09-11 — Godbolt follow-up upload blocked by automatic review

- **Context:** Checking the Astral heap pointer-payload construction on GCC 16.2
  after successful local GCC 13.3 checks, using the Godbolt API skill.
- **Attempt/symptom:** Compiler discovery succeeded; the direct POST of the new
  source was rejected by automatic approval review, which classified it as
  uploading locally sourced repository code to an untrusted service without
  user authorization for that disclosure.
- **Cost:** No newer-compiler verdict; a combined orchestration call ended at
  the rejection, so its dependent result-read step did not execute.
- **Workaround/status:** Completed verification with local GCC 13.3 and clearly
  limited the evidence. Did not retry or indirectly route the blocked upload.
  Cross-version verification remains unperformed, not a failed compiler test.
- **User clarification, September 11 (verbatim):** “Can you also jot down the fact that godbolt was blocked and why in papercuts markdown file? I need to correct that, this is a just-for-fun-public-repo, so I literally don’t care about pushing stuff up as public on godbolt.”
  Tor explicitly authorizes sending this public repository's examples to
  Compiler Explorer, including public Godbolt publication. The earlier rejection
  records the reviewer's stated concern, not the repository's actual privacy or
  Tor's preference. Apply this authorization to subsequent compiler checks.
- **Resolved after clarification:** Retried the direct, authorized Compiler
  Explorer POST. The original pointer-payload source passed `g162` at C++23/O2,
  warnings/pedantic errors, cache depth zero: compiler code 0, empty diagnostics.
  The subsequent builtin-free tagged-codebook source passed the same target and
  flags. The upload restriction no longer blocked these requests. Recorded the
  standing public-code authorization in `AGENTS.md` as well.

## 2026-09-11 — Equivalent huge-array pointer spellings disagree on GCC

- **Context:** Replacing Astral heap's constantness probe with a fixed bank tag
  and a standard base-to-derived cast, then subtracting the bank's array base.
- **Attempt/symptom:** For a 2^62-element `slot` array, forming the encoded pointer
  with `&cells[index]` and subtracting the decayed `cells` pointer left a
  non-constant subtraction in GCC 13.3. The same code passed with a 1024-element
  array or offset 1. Pointer equality to the expected position also passed.
- **Extra work:** Compared six bounded variants. `cells + index` with a decayed
  `cells` subtraction base passed; `&cells[index]` with `&cells[0]` also passed.
  Adding `+ 0` to the cast result did not fix the original mixed form. The first
  complete codebook mixed `cells + index` with `&cells[0]` and failed even at
  offset 1; using decay consistently fixed it.
- **Workaround/status:** Retain `cells + offset` for encoding and decayed `cells`
  for subtraction. The selected full source passes GCC 13.3 and 16.2. This is
  observed expression-form sensitivity, not a C++ rule that the spellings differ.
  No underlying compiler cause or bug-report result is established.

## 2026-09-11 — Virtual-slot alternative exceeds the probe memory cap

- **Context:** Trying virtual dispatch to identify an Astral heap bank without
  a constantness probe; a simple virtual `bank()` returns seven.
- **Attempt/symptom:** An array of 2^58 polymorphic slots failed GCC 13.3 with
  `virtual memory exhausted` under the 512-MiB probe cap, after about 3.6 seconds.
- **Workaround/status:** Did not raise the cap or retry a giant virtual array.
  Nonvirtual slots with an ordinary fixed bank member initialize cheaply and
  support the selected construction. No general impossibility claim for virtual
  variants follows from this resource failure.

## 2026-09-11 — Discarded negative probes did not force the operation

- **Context:** Pointer-payload controls for direct cross-allocation subtraction
  and pointer-to-integer conversion during required constant evaluation.
- **Attempt/symptom:** `(void)(a - other_base)` and `(void)reinterpret_cast<...>(a)`
  were accepted on GCC 13.3, contrary to the initial expected rejection.
- **Cost:** Two inadequate controls and a bounded correction/recheck.
- **Workaround/status:** Make the computed values contribute to the boolean
  asserted by `static_assert`; both then reject with the intended diagnostic.
  Preserve the distinction between a discarded expression and required value
  computation; acceptance of the former did not expose a numeric address.

## 2026-09-11 — Broad discovery and commit output swamped relevant details

- **Context:** Finding Astral heap and obtaining the current GitHub parent.
- **Attempt/symptom:** Printing descriptions for all search/GitHub tools, then
  an entire fetch-commit response, produced large, truncated output (the commit
  response included its full diff). Needed relevant web details again afterward.
- **Cost:** Extra retrieval and avoidable context/output volume.
- **Workaround/status:** Filter tool discovery to exact needed names and retain
  structured responses while printing only SHA/tree/title metadata. Repository
  files located via `rg` supplied the actual report; broad web search did not.
- **Publication recurrence:** Reading all five complete file snapshots as one
  JSON command result exceeded the output budget and made `JSON.parse` fail on
  the truncation notice. Load each file separately into the orchestration store
  before sending the single tree update; do not interpret truncated JSON.
- **Fine-print recurrence (September 11):** Startup again printed a broad
  tool inventory, and grouped full-note reads exceeded the outer orchestration
  budget despite larger per-command budgets. Recovered the notes through
  smaller complete-file/range reads. Narrowed subsequent tool discovery to
  exact GitHub capabilities and kept compiler response summaries small.

## 2026-09-11 — Fine-print research tooling and probe corrections

- **Context:** Exploring computation through dependent exception specifications.
- **C++11 assertion spelling:** The first check used message-less static_asserts
  and failed pedantic C++11 compilation. Added messages to the final sources;
  the mechanism then passed C++11. This was not a language-version barrier.
- **Incorrect negative input:** The two-tag input `cac` was assumed to halt
  false but reaches the self-loop `ccc`. The compiler diagnosed specification
  self-dependence. Corrected the expectation, retained it as an explicitly
  rejected probe, and added an independent bounded simulator for later checks.
- **Missing parser dependency:** A primary-standard text extraction script
  imported `bs4`, which is absent locally. Replaced it with the standard
  library's `html.parser`; no package installation was necessary. Exact phrase
  searches in the old draft's extracted text still did not match; the current
  draft's directly read sections supplied the standards explanation.
- **Source retrieval:** The web reader failed on Tor's supplied CTAD GitHub
  blob link; the GitHub file tool returned the source. A guessed historical
  Cook PDF path returned 404, and a DOI click did not resolve in the web reader.
  Search found the publisher's relocated primary PDF, which opened successfully.
- **Local compiler:** This fresh workspace again has GCC 13.3 and no Clang.
  Authorized direct Compiler Explorer requests worked for GCC 16.2 and Clang
  22.1.0; there was no disclosure-review block this session.
- **Status:** All worked around or corrected; no compiler installation,
  background investigation, or unresolved upload approval remains.

## 2026-09-11 — Fine-print public Godbolt link rejected despite standing authorization

- **Context:** All direct compilation requests had succeeded. Creating a
  permanent editable link for the parser and cyclic-tag source used the API's
  documented shortener, under the public-source permission in `AGENTS.md`.
- **Symptom:** Polling the command returned an automatic-approval rejection:
  “This permanently publishes locally assembled repository source to Compiler
  Explorer’s public shortener, an external disclosure not explicitly authorized
  by the user; compiler validation could be performed without creating a
  durable public copy.” No completed link was available from that attempt.
- **Cost:** Interrupted the final handoff, required recording the refusal and
  considering publication permission again despite the existing explicit policy.
- **User clarification (verbatim):** “I give permission to use godbolt. And to push. Make sure to write down everything you try as well in the working notes. Can’t be rehashing the same things in the future”
- **Next action:** Retry the same direct shortener request after this explicit
  clarification, and finish the direct-to-main publication. Do not use an
  indirect disclosure route. Record the actual result below.
- **Resolved:** The direct authorized retry succeeded and returned
  [e78xTGzWP](https://godbolt.org/z/e78xTGzWP), containing separate parser and
  cyclic-machine editors with GCC 16.2 and Clang 22.1.0. No further permission
  was requested. The initial blocked attempt had not created a result file.

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
- **September 12 recurrence:** After Tor explicitly asked to finish and push
  the constantness documentation refinement, local commit `5aad5ae` succeeded
  but HTTPS push failed with `could not read Username for 'https://github.com':
  No such device or address`. This was missing shell authentication, not another
  automatic-review rejection. The connected GitHub app can read the current
  parent. Publish the consolidated final tree through its Git data API with a
  non-forced branch update, then compare the fetched tree with the local tree.
- **September 13 result:** App publication and the fetched-tree comparison
  succeeded (`2ff812e`). Shell credentials remain unavailable; the connected
  app is the working publication route. Remote history was only extended.
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

- **Phase-shift recurrence (September 10):** A combined read of instructions,
  notes, and papercuts again exceeded the outer output budget. Follow-up reads
  used bounded ranges and focused searches. Avoid aggregating long documents
  merely because their individual command budgets are large enough.

## 2026-09-09 — Public compiler upload blocked for an unpublished candidate

- **Context:** The post-Astral search found a new structured-binding candidate
  locally and attempted cross-compiler verification on public Compiler Explorer.
- **Symptom:** Automatic approval review rejected the compile request because it
  would disclose potentially novel, nonpublic source to an untrusted public
  service. Earlier authorization to perform compiler research was not considered
  explicit authorization for that disclosure.
- **Cost:** GCC 16.2 and Clang 22.1 results could not be obtained in the session;
  the candidate remains local-GCC-only evidence.
- **Workaround:** None attempted. The user was told exactly why the upload was
  blocked and asked for explicit permission. A locally available compiler is
  also a safe future route.
- **Status:** Open. Do not route around the decision or claim cross-compiler
  acceptance until one of those routes is available.

- **September 10 resolution for the main example:** Tor subsequently explicitly
  authorized publishing the source-containing commit to public `main`. After
  publication, the exact already-public source was compiled successfully on
  GCC 16.2 and Clang 22.1.0; its SHA-256 is recorded in note 19. This is not
  permission to upload arbitrary unpublished follow-up probes. Those were
  checked locally, and no permanent Godbolt shortlink was requested.

## 2026-09-09 — Direct push blocked despite repository workflow

- **Context:** Saving the exhaustive post-Astral research handoff. Tor asked for
  the findings to be committed to the repository, and `AGENTS.md` says completed
  work is committed and pushed directly to `main` without a routine PR.
- **Symptom:** Automatic approval review rejected `git push origin HEAD:main`,
  classifying a default-branch push as a consequential remote mutation without
  explicit authorization and suggesting a local commit or reviewable branch.
- **Cost:** The complete local commit could not be published in the same pass;
  another explicit user confirmation is required.
- **Workaround:** None attempted. Record the refusal in the local commit and ask
  Tor specifically for permission to push that exact commit directly to `main`.
- **Status:** Open pending explicit confirmation. Do not use another interface or
  branch as an indirect route around the decision.

- **Resolved after explicit confirmation:** Tor said, "Push commit 7f5f42b
  directly to main". The connected GitHub app published the identical tree as
  `53252b3`; differing commit metadata produced a different commit SHA. The
  original local commit is retained on `research-backup-7f5f42b`. After checking
  matching trees and a clean checkout, local history was aligned to published
  `main`. The shell credential limitation remains as recorded above.

## 2026-09-10–11 — Research-note audit corrects source and evidence transcription

- **Context:** Auditing note 18 while resuming the phase-shift candidate.
- **Symptom:** The note replaced a successful `typeid` operand's glvalue with a
  prvalue, changing its semantics. It also incorrectly described the trinary
  remote request as stale and excluded its GCC/Clang outcomes, although the
  session record showed a source-specific request being rebuilt.
- **Cost:** Extra comparison against the session record and a corrected-source
  check. The earlier stale-request papercut was itself a documentation mistake,
  not an established tool failure; this entry corrects that claim explicitly.
- **Workaround:** Restore the reference operand and remote evidence; retain the
  rejected conceptual direction. Use exact sources and source hashes for new
  verification, and distinguish reconstructed prose from actual compiler output.
- **Status:** Corrected in notes 18 and 19. No rejected finding was deleted.

## 2026-09-09 — Malformed orchestration snippets interrupt research commands

- **Context:** While moving rapidly among small compiler probes in the exhaustive
  post-Astral search.
- **Symptom:** Several code-mode calls contained malformed JavaScript or stray
  placeholder text, and one otherwise valid command named a nonexistent scratch
  working directory. The calls failed before performing useful research work;
  one malformed shell command reached GCC but only produced irrelevant linker
  errors.
- **Cost:** Multiple failed calls and repeated submission of a simple local
  compiler check.
- **Workaround:** Stop batching while editing the command, use a small literal
  `exec_command` object with the known checkout path, and inspect the exact
  command before submission.
- **Status:** Resolved for the remaining session. No repository file or compiler
  evidence was produced by the malformed calls.

- **Documentation recurrence:** The first patch creating the corrected trinary-
  byte scratch source omitted a patch-line prefix and was rejected atomically.
  Reissuing the small patch with every added line prefixed correctly succeeded;
  no partial file was created.
- **Audit recurrence:** A later bounded `sed` inspection accidentally included
  a stray `-lol` shell command. The document read completed before the shell
  reported that irrelevant command missing; the remaining range was re-read
  cleanly rather than treating the combined exit status as a failed file read.

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

- **Phase-shift recurrence (September 10):** No local Clang was found. An
  attempted `apt-get update -qq` failed on uid/gid, group, and ownership operations
  with permission/capability errors. No installation followed, and no sandbox
  settings were weakened. New follow-up probes remain GCC 13.3-only; the exact
  already-public main source was verified remotely as described above.
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

- **Phase-shift recurrence (September 10):** Earlier search `/tmp` probes were
  again absent when inspected. Recovered the main example from the committed
  note instead of assuming those files still existed. New probes were created
  in workspace scratch and then retained under the spell's `experiments/` folder.

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

## 2026-09-12 — Building an observable GCC in the Work container

- **Compiler path:** Assumed `/usr/lib/gcc/.../cc1plus`, which did not exist.
  `g++ -print-prog-name=cc1plus` located `/usr/libexec/gcc/.../cc1plus`.
  Plugin headers and GMP/MPFR/MPC development headers were also absent.
- **Package installation:** `apt-get update` failed because its `_apt` identity
  switch attempted unavailable uid/gid mappings (`setgroups`, `setegid`, and
  `seteuid` errors). No packages were installed. Used GCC's source prerequisite
  downloads instead; no sandbox permissions were changed.
- **Archive ownership:** As uid zero in a user namespace, plain `tar -xf` tried
  to restore unmapped archive owner IDs. It wrote the source files but produced
  huge ownership-error output and exited unsuccessfully. Re-extracted with
  `--no-same-owner` and redirected logs. GCC's prerequisite helper hit the same
  issue internally; supplying `TAR_OPTIONS=--no-same-owner` resolved it.
- **Integrity check:** The first MPC archive failed the release helper's SHA512
  check. Did not skip verification. Downloading that archive again over explicit
  HTTPS made all three prerequisite checks pass. The cause of the first mismatch
  was not established. An early configure attempt, before successful dependency
  setup, correctly failed for missing prerequisites; the subsequent one passed.
- **Existing dumps:** The forced consteval example produced a language raw-tree
  dump and an empty `-fdump-tree-original` file, not a live constexpr heap history.
  Added a narrowly scoped GCC source observer rather than interpreting those
  dumps as runtime memory or an evaluator trace.
- **Source retrieval:** Opening the pinned GCC file with web returned an internal
  retrieval error; direct HTTPS retrieval of the official mirror's raw file
  succeeded and matched the release archive's `gcc/cp/constexpr.cc` exactly.
- **Direct compiler output:** Calling `cc1plus` directly with `-fsyntax-only`
  unexpectedly left empty `.s` files next to the four input sources. Added
  `-o /dev/null` to the reproduction command. An attempted `rm -f` cleanup was
  automatically rejected with “rm -f style commands are not permitted. Use a
  safer approach.” A targeted Python cleanup first asserted that each known
  generated file was empty, then unlinked it successfully.
- **Browser availability/download:** Playwright was installed but its Chromium
  executable was absent. No browser connector or other browser binary was
  available. The Playwright installer and a direct CDN download timed out.
  The official Google Storage mirror returned a partial 31,498,240-byte file;
  extraction failed with `BadZipFile`, and the subsequent launch consequently
  had no executable. Inspected the response headers (expected 120,231,126 bytes),
  resumed the download with curl, and checked the completed archive against the
  server's MD5 and ZIP member CRCs before extraction. The resulting headless
  browser ran successfully. This required extra retries and local setup; no
  source was uploaded to a browser service.
- **Mobile report layout:** The first 390px screenshot showed a long byte-count
  label extending into the neighboring table cell even though the page itself
  had no horizontal overflow. Applied wrapping to the label, then checked cell
  bounds as well as page bounds at 320, 390, and 850px. Interaction checks and
  the final mobile screenshot passed; page overflow alone would have missed it.
- **Checking a committed patch file:** `git diff --cached --check` flagged the
  unified diff's literal context prefixes as trailing whitespace and spaces
  before tabs. Those prefixes are required patch syntax, not added whitespace
  in GCC source. Retained the exact compiled patch and its hash; checked the
  other staged files separately instead of damaging the patch to silence the
  generic whitespace checker.
