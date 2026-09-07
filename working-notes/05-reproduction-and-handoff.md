# Reproduction and handoff

## Selected sources and compiler settings

The three root spell sources are the selected artifacts. Their initial published
version is commit `8864a48f0988c9eab0831088d7ade14b5969621c`. Eight saved checks
bound those files by SHA-256 to C++23 compilations: four vector configurations,
two GCC counter configurations, and two Clang empty-bits configurations, all
accepted. Those hashes were checked again while preparing these notes.

Compiler Explorer discovery selected x86-64 GCC 16.2, ID `g162`, and Clang
22.1.0, ID `clang2210`. These are the session versions, not a claim about the
latest available compilers when this is read later.

| Group | Language / options |
| --- | --- |
| Initial span, tagless union, original integer vector | C++20, `-O0` and `-O2` on both |
| Template vector development | C++20; full selected source also checked in C++23 |
| Root spells | C++23, `-O0` and `-O2`, on their advertised compilers |
| Compiler-state exploration | C++23, primarily `-O2`, with counter/empty `-O0` checks |
| Early pointer probes | C++23; macros select routes |
| Broader pointer sweep | Baseline C++23 `-O2`; selected extra configurations in the pointer notes |
| Compile-time cat | `-std=c++2c`, `-O0` and `-O2`, intentional diagnostic failure |

The cache controls used GCC `-fconstexpr-cache-depth=0` and
`-fconstexpr-depth=1024`. The sentinel investigation also attempted
`-fconstexpr-loop-limit=64`; Clang does not accept that option. Four selected
pointer cases used Clang `-fexperimental-new-constant-interpreter`; the rest did
not enable it. Do not combine outcomes from different interpreter settings.

## What was checked for this handoff

The scratch workspace held three original reports, 19 C++ probe snapshots, four
request scripts, and 160 retained compiler records: 44 older loose responses,
8 source-hashed root checks, 26 early pointer checks, and 82 broader pointer-sweep
configurations spanning 36 program names. These were inspected to reconcile the
working notes. They are not a count of every request made during the session.

Per Tor's clarification, this commit contains the working documents, without a
raw archive of those files. The important results, controls, limitations, and
existing editable links are recorded here. Do not assume an ephemeral workspace
will still have the raw records in a future session.

Some early scratch filenames were reused, especially `one-pointer-vector.cpp`.
Use the original Godbolt source for the first integer vector and the root file
for the selected template; a loose response's filename alone does not prove that
it compiled the last file with that name. The root hash-bound checks and later
pointer records had stronger source provenance. No new compile requests were
made for the documentation pass.

Successful HTTP responses are not successful compilations. Some negative probes
failed their intended assertions, some failed because an option or builtin was
unsupported, and some timed out. The notes distinguish these. The nested-union
byte was a useful correction recovered from the saved results: its compile-time
checks pass while its ordinary demonstration returns 57.

## Reproduce a selected spell

Use the root source's own `static_assert`s and the named compiler. Compile to an
object; these files do not require a `main`:

```sh
g++-16 -std=c++23 -O2 -c one-pointer-vector.cpp -o /tmp/one-pointer-vector.o
clang++-22 -std=c++23 -O2 -c empty-bits.cpp -o /tmp/empty-bits.o
g++-16 -std=c++23 -O2 -c cache-counter.cpp -o /tmp/cache-counter.o
```

Executable names depend on the local installation. If using Godbolt, read the
`use-godbolt-api` skill and current
[Compiler Explorer API documentation](https://github.com/compiler-explorer/compiler-explorer/blob/main/docs/API.md),
discover the compiler ID, submit the source and flags, and inspect compiler
`code`, diagnostics, and assembly. Force the claimed constant evaluation.
None of the session's observations are runtime benchmarks.

For later probes, the [pointer notes](04-pointer-investigation.md) contain the
scalar wrapper, integer round trip, cast loophole, and decisive controls.
Keep fake `std` declarations isolated from actual library headers. Reproduce one
claim at a time, preserving language mode, optimization, and interpreter choice.

## Existing editable links

| Example | Godbolt |
| --- | --- |
| Pointer-only span | [474oesPvP](https://godbolt.org/z/474oesPvP) |
| Array/aggregate boundaries | [5he7s1xKq](https://godbolt.org/z/5he7s1xKq) |
| Compile-time cat | [ja4js9bj7](https://godbolt.org/z/ja4js9bj7) |
| Tagless number union | [1bazeoePG](https://godbolt.org/z/1bazeoePG) |
| Original integer vector | [6f867K31h](https://godbolt.org/z/6f867K31h) |
| Compact templated vector sketch | [MscKzxdjf](https://godbolt.org/z/MscKzxdjf) |
| First side-effect probe | [PPzz6K86r](https://godbolt.org/z/PPzz6K86r) |
| Cache counter | [j6d7b6nEn](https://godbolt.org/z/j6d7b6nEn) |
| Cache controls | [v3MT7f8xG](https://godbolt.org/z/v3MT7f8xG) |
| Empty bits | [cxqejscT1](https://godbolt.org/z/cxqejscT1) |
| Recursion headroom | [TcsWEdrG1](https://godbolt.org/z/TcsWEdrG1) |

The compact vector link uses C++20 and shorter checks than the full C++23 root
source. These links identify earlier snapshots, not automatically the current
root file.

No new public Godbolt links were confirmed for the later pointer/empty-value
probes. Automatic approval review rejected an attempt to publish two new snippets
because permanent publication had not been authorized for those sources. Do not
invent links or assume a shortener request succeeded. The later request to
commit these working documents is the scope of this handoff.

## Repository workflow and future work

Public GitHub fetch/clone worked during this session; HTTPS `git push` lacked
credentials. The connected GitHub app published the first spells using a tree,
commit, and non-forced branch update. If that is still the available path,
read the current remote head, build on its tree, preserve existing history, and
verify the resulting commit. Never force-push the unpublished preparation root.
Authentication availability can change; this is historical workflow context.

Keep the handoff here and the root README focused on the spells. Root
`AGENTS.md` points future collaborators at this directory. Avoid adding CI,
dependencies, generic APIs, reflection, or a large test framework without a
concrete new request. For a changed spell, verify its claimed behavior on its
named compiler.

If Tor chooses to resume, independent possibilities include:

- Refine the fake allocator/source-location cast as a small spell, preserving
  its type and interpreter restrictions.
- Develop the nested-union byte, retaining its forced/ordinary-call distinction
  and finite depth bound.
- Refine scalar `empty<T>` without claiming arbitrary type support.
- Investigate a particular compiler discrepancy with a pinned source revision
  and minimal reproduction, if Tor wants a bug report.
- Revisit pointer storage only at Tor's direction. The empty owning vector is
  tabled; the failed families do not establish impossibility.

These are possibilities, not commitments or work running in the background.
