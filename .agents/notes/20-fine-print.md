# Fine print — computation in exception specifications

September 11, 2026. New search after reading all 19 existing research notes,
the catalog, Pedantics, and papercuts. Starting remote main: `9d731544`.
This is an incremental research record; only explicitly reported checks are
evidence. The final source and compiler matrix will be recorded below.

## Scope

Tor clarified this branch's target verbatim:

> To be clear, specifically “compile time c++ can do that”

Later publication and research-retention clarification, verbatim:

> I give permission to use godbolt. And to push. Make sure to write down everything you try as well in the working notes. Can’t be rehashing the same things in the future

Keep the existing mechanisms and rejected initializedness family excluded from
the new headline. The tabled empty owning vector stays tabled. Read Tor's earlier
CTAD merge-sort source too; its intermediate `.value` extraction remains a
different, unresolved investigation.

## Search and first result

Considered member-lookup dominance and ambiguity as composable logic, ADL as
graph traversal, implicit conversion selection, pointer qualification as set
operations, and implicit special-member exception inference. These were thought
experiments, not compiled results. None was selected: basic reachability, a
single lookup gate, or another conjunction-only special-member property was too
weak to establish a new machine. Do not describe these as proven impossible.

The productive question was whether lazy exception-specification instantiation
can supply recurrence after overload selection, without recursive return types
being examined in unselected candidates.

A balanced-parentheses parser made only of `void` function declarations passed
GCC 13.3/C++20/O0 and Compiler Explorer Clang 22.1.0/C++20/O2, with warnings and
pedantic errors. Its two recursive overloads push or pop a character pack; their
exception specifications ask `noexcept` about the next configuration. Empty
input plus empty stack accepts; an ellipsis fallback rejects. There are no
user-written function bodies, constexpr functions, or intermediate member reads.

The allowed mechanism is explicitly **function-template deduction and overload
selection, pack substitution, and lazy exception-specification propagation**.
Do not silently call this noexcept alone or a solution to pure overload-only
recurrence. The declarations encode rewrite rules; empty class templates are
inert type containers. `noexcept` propagates the terminal Boolean through the
selected transitions.

## Subsequent probes so far

- A two-tag-system sketch with rules `a -> ccbaH`, `b -> cca`, `c -> cc`
  accepted the trace from `baa` to an `H`-headed word. Short non-H words reject.
  One attempted negative input `cac` was mistakenly assumed to halt: it reaches
  `ccc`, which rewrites to itself. GCC diagnosed a self-dependent exception
  specification. This is **not** a false result or a halting oracle. Replaced
  that expectation with the genuinely terminal one-letter `b`.
- A lazy-selection control gives a generic candidate an invalid dependent
  exception specification, while an exact `int` overload is noexcept. Querying
  the exact overload succeeds. A second control leaves an infinitely growing
  recursive generic candidate unselected when a non-template empty-list
  overload wins. Both controls pass local GCC 13.3/C++17/O0 with constexpr
  call depth 1 and template depth 128.
- The first C++11 probe used message-less static_asserts; pedantic compilation
  correctly rejected that C++17 syntax. This was an assertion spelling issue,
  not a C++11 rejection of the exception-specification mechanism.
- A generic cyclic-tag interpreter uses a character-pack queue and a type-pack
  ring of production words. On 0 it deletes the head; on 1 it also appends the
  active production. Both rotate the production ring. It passes local GCC
  13.3/C++17/O0, constexpr depth 1, template depth 128, including cycling and
  optional Y/N terminal markers producing opposite answers. Its initial ring
  `[1, empty]` did not grow the queue; the retained `[11, empty, empty]` program
  adds that stronger control.

All local checks above used `-Wall -Wextra -pedantic-errors -fsyntax-only`.
The early Clang parser response is bound to SHA-256
`e92a04b10bf34685ebc2ca12c7335230c943bc35114d69323a6058923558eba2`.

## Final selected sources and assessment

- [Fine print](../../tricks/fine-print/fine-print.cpp) retains the small parser,
  with the [walkthrough](../../tricks/fine-print/README.md) building from ordinary
  noexcept queries to recursive specifications.
- [Cyclic tag](../../tricks/fine-print/cyclic-tag/cyclic-tag.cpp) is a generic
  interpreter. Unlike the earlier two-tag probe, its program is data: a pack
  of production-word types. Its declarations remain fixed across programs.
- [Controls](../../tricks/fine-print/experiments/controls.cpp) and
  [two-tag history](../../tricks/fine-print/experiments/two-tag.cpp) preserve
  the substantive earlier probes, including the wrong `cac` expectation behind
  a deliberately failing macro.

The queue/ring configuration is carried solely in template argument packs.
Overload deduction distinguishes 0 and 1. Pack substitution removes the head,
conditionally appends a production, and rotates the rule ring. A dependent
exception specification asks about the next configuration. Three overloads
implement the binary cyclic model; two optional Y/N terminal overloads permit
observable true and false halts. No intermediate result is extracted into a
different evaluator. The empty type containers have no members or recursive
definitions. No user-written function is defined or evaluated.

Cook's primary paper, [Universality in Elementary Cellular Automata](https://content.wolfram.com/sites/13/2023/02/15-1-1.pdf),
section 2.2 (pages 7–8), was read. The binary C++ rules directly implement its
cyclic-tag transitions, giving a translation argument to a universal model.
The paper's ordinary alphabet Y/N is renamed 1/0 here; our optional Y/N halts
are separate extensions. This meets Tor's three ingredients within the explicitly
combined allowed mechanism. It is not a noexcept-only or overload-only claim.
The supplied small programs are not asserted to be universal themselves; no
general machine compiler or arbitrary final-tape decoder was implemented.

The result updates `docs/MACHINES.md`: a complete combined construction is now
demonstrated, while the earlier isolated-single-feature questions stay open.
The parser alone has only pushdown-machine power; that was why the generic
queue-rewriting application was worth checking before updating the status.

## Standards and a control correction

Read current draft [except.spec](https://eel.is/c++draft/except.spec),
[temp.inst](https://eel.is/c++draft/temp.inst), and
[expr.unary.noexcept](https://eel.is/c++draft/expr.unary.noexcept).
Exception specifications are separately instantiated when needed; selection
by overload resolution makes one needed. A noexcept operand is unevaluated,
although determining its exception specification can instantiate further ones.
Dependent unqualified calls use ADL to find later transition declarations in
the same namespace as the configuration types. All declarations precede the
first actual query; nothing changes definition or satisfaction over time.

The early growing-recursion lazy control had no demonstrated valid generic
specialization. Merely leaving such a template unselected risks the no-valid-
specialization rule. The retained control adds a terminating `list<int, char>`
overload and successfully queries the generic `list<char>` specialization.
The unselected `list<>` generic route would still grow forever, while the exact
empty-list overload wins safely. A second control similarly demonstrates a
valid specialization of the dependent-member candidate. The retained claim
rests on these corrected controls, not the earlier all-divergent sketch.

The parser's fallback false is a valid call expression, as is the cyclic
machine's N halt. A separate type check confirms the latter still has type void.
Ill-formed selected specifications do not participate in a recoverable SFINAE
search. Self-dependence and resource exhaustion are not false return values.

## Final compiler evidence

Local compiler: Ubuntu GCC `13.3.0-6ubuntu2~24.04`. Remote IDs freshly discovered:
`g162` (x86-64 GCC 16.2), `clang2210` (x86-64 Clang 22.1.0).
All positive runs returned code zero with no diagnostics.

| Check | Configuration | Result |
| --- | --- | --- |
| Four selected C++ files | GCC 13.3, C++11/O0, warnings/pedantic, constexpr depth 1 | Pass; final controls also pass at template depth 48 |
| Same four files | GCC 13.3, C++23/O2, warnings/pedantic, constexpr depth 1, cache depth 0 | Pass |
| Four sources combined in independent namespaces | GCC 16.2 and Clang 22.1.0, C++11/O2, warnings/pedantic, constexpr depth 1, template depth 128 | Both pass |
| Independent generated checks | GCC 13.3 C++11/O0; GCC 16.2 and Clang 22.1.0 C++11/O2; same warnings and depth 1/128 | All pass |
| Main parser object, `nm` inspection | GCC 13.3, C++11/O0, warnings/pedantic | Compiles; no symbols emitted |

The independent [checker](../../tricks/fine-print/experiments/check.py) checks
all 511 parenthesis strings of lengths zero through eight against a depth
counter. It then enumerates 81 two-rule programs over nine possible productions
and 15 binary inputs per program, using an ordinary deque and numeric program
counter. Among 1,215 cyclic cases it obtains 792 true halts and 148 false halts.
It excludes 205 repeated configurations and 70 runs exceeding 80 transitions;
neither category is declared false. Thus 1,451 generated Boolean assertions
are checked, in addition to the source's handwritten controls.

Five deliberately rejected local controls use C++11/O0, warnings/pedantic,
constexpr depth 1, template depth 48, and `-DPROBE=N`:

| N | Outcome |
| --- | --- |
| 1 | Exact configuration cycle: exception specification depends on itself |
| 2 | Increasing queue: template instantiation depth exceeds 48 |
| 3 | Unknown symbol x: no matching run overload |
| 4 | Empty production ring with nonempty queue: no matching run overload |
| 5 | Selected invalid dependent-member specification: char is not a class |

Each returns compiler code 1 for the stated primary diagnostic. Any following
failed assertion is diagnostic fallout, not a successfully obtained false.
No remote negative-control results are claimed.

| Source | SHA-256 |
| --- | --- |
| `fine-print.cpp` | `10982326dda23ed3aad1f03067a3a99e84b5936cbdc70c4b3fe074640f2cbf28` |
| `cyclic-tag.cpp` | `9a6a8d65da69e2d911acfc16d31d1130b97ff05dfe461eb61cf072307eea3d64` |
| Four-source remote bundle | `b0f6644838864ee7f4d5be07cd6e4de649171b4a62218a9b54f57c4c0f5b326e` |
| Generated remote suite | `c74f5d7690ac9678aae9dd52dd7aa6497f6b0a80312c20b51e33b0e009aac0b2` |

Remote response records retained source, SHA-256, arguments, compiler code,
and diagnostics in scratch. The reproducible code and evidence summary are
committed, without adding a raw-response archive or build framework.

The [public editable Godbolt](https://godbolt.org/z/e78xTGzWP) contains both
selected examples with GCC 16.2 and Clang 22.1.0. Automatic approval review
rejected the first shortener attempt despite standing repository permission;
the same direct request succeeded after Tor's explicit clarification above.
The refusal, stated reason, and resolution are preserved in PAPERCUTS.
The shortlink-info API was read back: both saved sources exactly match the
repository files, with the two intended compiler IDs. Final relative-link and
Git whitespace checks passed.

## Provenance and stopping point

Independent construction by Tor Shepherd and Codex. Conditional noexcept,
template rewrite systems, and separate instantiation are established language
features; cyclic tag systems are Cook's work. Targeted searches for noexcept
plus Turing-completeness, interpreters, parsers, tag systems, and bodyless
computation did not establish an exact prior match. Search results were often
broad or unrelated. No historical-first claim follows.

The finite parser and generic cyclic machine complete this branch's discovery.
Possible future work, not pending tasks: a useful arithmetic program in the
cyclic dialect, alternative output conventions, or an even narrower feature
boundary. Do not automatically grow this into a metaprogramming framework or
claim that the pure-overload recursion question has been resolved.
