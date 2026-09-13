# Removing `__builtin_constant_p`

September 12, 2026. Tor asked whether the builtin-dependent tricks could instead
use `if constexpr` or `std::is_constant_evaluated()`, specifically mentioning the
one-pointer span and empty integer. He added, verbatim:

> I also give permission to upload to godbolt

The walkthroughs now live with [Empty bits](../../tricks/empty-bits/),
the [one-pointer vector and span](../../tricks/one-pointer-vector/), and the
[cache counter](../../tricks/gcc-memoization/counter/). This is a bounded audit
of existing mechanisms, not a new spell family. The original sources and their
C++23 evidence are preserved; verified alternatives are companion sources.
The empty owning vector remains tabled. No claim of historical novelty or
isolated-machine power is made.

## Starting state and sources

Read the checkout at `124603e`, root instructions, handoff index, relevant
span/compiler-state notes, and current empty-bits/vector/cache/Astral sources.
The tagged Astral codebook was already builtin-free: no new implementation or
retest of that construction was needed.

Consulted [GCC's builtin documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html),
the current draft's [stmt.if](https://eel.is/c++draft/stmt.if),
[meta.const.eval](https://eel.is/c++draft/meta.const.eval), and
[type requirements](https://eel.is/c++draft/expr.prim.req.type), plus
[P2641R4](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2641r4.html)
and [P3450R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2026/p3450r1.html).
P2641 explicitly considers general constant-evaluation failure detection and
chooses the narrower lifetime query. P3450 extends the query to test casts;
that extension was read but not used here. LLVM's current
[`builtin-is-within-lifetime.cpp`](https://github.com/llvm/llvm-project/blob/main/clang/test/SemaCXX/builtin-is-within-lifetime.cpp)
explicitly expects null and one-past pointers to be rejected. Direct CE probes
confirmed the one-past result on both supported toolchains.

Compiler IDs were obtained from CE's live `/api/compilers/c++` list, not guessed:
`g162`, `clang2210`, `gsnapshot`, `clang_trunk`. The GCC snapshot's diagnostics
identify `gcc-trunk-20260912`, library version 17.0.0. Local GCC is 13.3.0;
this session's compiler evidence comes from CE. All POSTs succeeded as HTTP
operations; nonzero compiler outcomes below are actual compilation results.

## Every tested direction

1. **Original span and direct substitutions.** The retained
   [constantness-controls.cpp](../../tricks/one-pointer-vector/experiments/constantness-controls.cpp)
   tests `{10,20,0,40}`. `PROBE=0` passes GCC 16.2 and Clang 22.1.0, C++23/O2.
   `PROBE=1` (`is_constant_evaluated`), 2 (`if consteval` plus an unconditional
   scan), and 4 (simple `requires { *p; }`) all reject on a one-past read.
   `PROBE=3` (`if constexpr (*p == 10)`) rejects because the ordinary parameter
   is not a constant expression for that condition. These are deliberate
   negative controls, not failed implementations left for repair.
2. **Template pointer and index.** The stronger type requirement
   `typename std::integral_constant<int, P[I]>` yields false on substitution
   failure. The retained [template-span.cpp](../../tricks/one-pointer-vector/experiments/template-span.cpp)
   counts four elements on both compilers, C++23/O2. It changes the interface
   to template arguments; it cannot read arbitrary evolving caller-local
   state or constexpr allocations through ordinary pointer parameters.
3. **Standard lifetime API availability.** A two-member union switches `a`
   to `b` and checks both lifetimes before/after. GCC 16.2 and Clang 22.1.0's
   default library setup reject because `std::is_within_lifetime` is absent.
   GCC trunk, Clang 22.1.0 with libc++, and Clang trunk with libc++ pass in
   C++26/O2. The underlying `__builtin_is_within_lifetime` passes the same
   union test on Clang 22.1.0, Clang trunk, and GCC trunk; GCC 16.2 lacks it.
   This isolated front-end support from library exposure. The final examples
   use the real standard-library declaration, not a hand-written `std` shim.
4. **Empty integer substitution.** Replace both liveness probes by
   `std::is_within_lifetime(pointer)`, make constructor/get/set immediate, and
   make the test lambdas explicitly immediate. Keep the destructor constexpr.
   The first default-library submissions stopped at the missing API; those
   runs establish no lifetime semantics. With supported toolchains, Clang
   22.1.0/libc++ passes every original assertion; GCC trunk fails both value
   assertions (rather than lacking the API). Remove the now-unused `alive()`
   method and retest the exact retained source on Clang at O0/O2: both pass.
   Query standardization does not fix the original base-replacement and
   potentially-overlapping-object portability caveats.
5. **One-past lifetime query.** A consteval function creates `int a[2]` and
   returns `!std::is_within_lifetime(a+2)`. Both GCC trunk and Clang 22.1.0/libc++
   reject; Clang explicitly says the query cannot take a one-past pointer.
   This cannot simply replace the existing span's stopping test.
6. **Owning vector with an explicit lifetime sentinel.** Add `char end` to the
   union, activate it in the already-reserved final slot, count active `value`
   members for size, and scan to active `end` for capacity. The first successful
   size loop checked both `!alive(empty)` and `!alive(end)`; simplify it to
   `alive(value)` and rerun the exact retained source. Both forms pass GCC trunk
   and Clang 22.1.0/libc++, C++26/O2. The existing full tests cover all six
   element categories, full capacity, growth, popping, zero/null values, and
   reuse. No out-of-bounds lifetime query occurs. This changes the backing
   representation, so it is available to the owner, not a drop-in scan of an
   arbitrary existing plain array.
7. **GCC counter through a type requirement.** Move the scan index into a
   template argument and use a fresh `Unique` argument for each query. The
   type requirement tries `slow(N)`; its failure selects `slow(N,500)` to warm
   the cache. Bound the scan at eight keys so raising recursion limits cannot
   cause an endless search. The initial 0/1/2/3 checks pass GCC 16.2 and fail
   Clang 22.1.0 (all zero), C++23/O2. The final GCC source adds repeat-identity
   and fifth-value checks, and passes. Cache disabled plus `NO_CACHE` gives
   zero twice. Depth 1024 plus `DEEP_LIMIT` makes every tested cold key pass
   and returns the bounded -1 fallback. Both controls pass their expectations.
   There are no builtins in this source. It is still implementation-dependent
   cache storage. No dictionary/vector/iterator rewrite was attempted.

Do not overgeneralize the template result into a generic `can_consteval(f)`
that accepts arbitrary stateful function arguments: the staging boundary is
the central limitation. Likewise, a simple unevaluated requirement is not a
value-evaluation check. Ordinary C++ exceptions cannot catch undefined behavior
or the compiler's constant-evaluation diagnostic.

## Reproduction

All paths are from the repository root. Common flags used remotely were
`-O2 -Wall -Wextra -pedantic-errors`; later probes also disabled diagnostic
colors. The empty integer was additionally checked at O0. Use the matching
compiler executable for each command.

```sh
# GCC 16.2 or Clang 22.1.0; PROBE=0 passes, 1..4 intentionally reject.
g++-16 -std=c++23 -O2 -Wall -Wextra -pedantic-errors -DPROBE=0 -fsyntax-only tricks/one-pointer-vector/experiments/constantness-controls.cpp
g++-16 -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/one-pointer-vector/experiments/template-span.cpp

# Clang 22.1.0 with libc++; empty-bits value checks fail on GCC trunk.
clang++-22 -std=c++2c -stdlib=libc++ -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/empty-bits/empty-bits-cxx26.cpp
clang++-22 -std=c++2c -stdlib=libc++ -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/one-pointer-vector/one-pointer-vector-cxx26.cpp
clang++-22 -std=c++2c -stdlib=libc++ -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/one-pointer-vector/experiments/within-lifetime-controls.cpp
# Add -DONE_PAST to the preceding control to reproduce intentional rejection.
# GCC trunk passes the latter two commands without -stdlib=libc++.

# GCC 16.2, three distinct configurations with matching expectations.
g++-16 -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fsyntax-only tricks/gcc-memoization/counter/counter-requires.cpp
g++-16 -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-cache-depth=0 -DNO_CACHE -fsyntax-only tricks/gcc-memoization/counter/counter-requires.cpp
g++-16 -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fconstexpr-depth=1024 -DDEEP_LIMIT -fsyntax-only tricks/gcc-memoization/counter/counter-requires.cpp
```

The positive union test and one-past rejection were initially separate scratch
sources and are retained together behind `ONE_PAST`. The source bodies retain
the actual tested operations. No result depends on the continued existence of
scratch request/response JSON.

The authorized CE shortener succeeded:
[on8WTjYKq](https://godbolt.org/z/on8WTjYKq) stores the final empty integer,
lifetime-query vector, cache counter (including its no-cache configuration),
and template-pointer span in separate editors. No upload approval was blocked.

## Documentation refinement

Tor's follow-up, verbatim:

> Amazing nice. Can you go through and update the existing readmes for each trick with this info? I don’t just want to sprawl more, but rather refine the existing docs and examples showing both the builtin constant p version and these workarounds

Folded the explanations, paired snippets, source links, compiler comparisons,
and reproduction commands into the existing trick READMEs. The original span
and its restricted template version stay in the owning vector's walkthrough;
the cache counter owns its alternative-probe explanation. Cache memory, both
cache-vector applications, and Seance's crossover explicitly retain builtin
storage and link to the counter's interface limitation. Astral heap compares
its already-existing encodings at the start of the pointer-payload explanation.

Moved the three verified alternative sources beside their originals:

| Previous path | Current path |
| --- | --- |
| `tricks/empty-bits/experiments/within-lifetime.cpp` | `tricks/empty-bits/empty-bits-cxx26.cpp` |
| `tricks/one-pointer-vector/experiments/within-lifetime-vector.cpp` | `tricks/one-pointer-vector/one-pointer-vector-cxx26.cpp` |
| `tricks/gcc-memoization/experiments/requires-counter.cpp` | `tricks/gcc-memoization/counter/counter-requires.cpp` |

These are byte-identical moves, verified by SHA-256. The retained controls and
restricted template-span source remain under `experiments/`. Removed the
separate `docs/CONSTANTNESS.md` after integrating its explanation and keeping
all results and primary references here. Updated links and reproduction paths;
the existing public Godbolt examples still contain the identical source text.
No fresh compiler claim or algorithm change is introduced by this doc pass.

Verification passed for 71 relative links/anchors and 29 reproduction-command
source paths across the edited pages. All C++ source bytes match the preceding
commit, including the three moved alternatives. Whitespace checks pass. No
compiler reruns were needed for these source-identical moves and doc edits.
