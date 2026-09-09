# Cache memory — September 7, 2026

Tor invited Codex to choose between continuing experiments, discovering new
techniques/applications, and obtaining his Static Antics transcript. This session
chose an application of the existing GCC cache counter. The tabled pointer
investigation was not reopened, and no transcription was attempted.

## Selected result

[cache-memory.cpp](../../tricks/gcc-memoization/memory/memory.cpp) implements a mutable mapping from `int`
keys to unsigned 64-bit values. Separate top-level constant evaluations can
write, read, and overwrite the same key. An absent key reads as zero.

The source has three functions and no includes, templates, mutable globals, or
container object. Its three-integer cache key is `(key, revision, bit)`; the
fourth argument is recursion depth. This builds on `cache-counter.cpp` and its
observed cold/warm distinction, not on mutation inside a builtin probe.

The first scratch prototype was a write-once map: warm each set payload bit and
a presence bit, rejecting subsequent writes. It passed local GCC 13.3.0 checks.
Adding a revision number made overwrites possible without removing cache entries.
An early `remember` returned `void`; the selected version returns its supplied
value, making the checks smaller. Do not mistake that return value for evidence
of a fresh write: the replay check deliberately demonstrates otherwise.

## Encoding

Each completed revision has a warm position 64, even if the stored value is zero.
Positions 0–63 are warm exactly when their payload bits are set. Both public
functions scan completed revisions starting at zero. The writer fills the first
unmarked revision; the reader reconstructs the previous revision. For an absent
key, the reader probes revision −1, which the writer never uses.

No cache fact is erased. Values decrease or become zero by selecting a new
revision, not by cooling an old entry. This is an append-only history implementing
an overwritable interface. A key's revision scan is linear in its number of
writes, and every read probes 64 payload positions. The compiler retains the
actual storage, and its resource limits still apply.

## Cache identities and demonstrated traps

The unused default `__builtin_LINE()` arguments vary the public wrappers' cache
keys across the top-level call sites in the source. They are not payload or
revision numbers. In particular:

- An explicitly repeated `recall(9, 9000)` remains zero after writing 17 to key 9.
  A fresh `recall(9, 9001)` sees 17. A missing result can itself be cached.
- `remember(9, 23, 9002)` is followed by a fresh write of 31. Repeating the first
  exact call returns 23 but does not restore 23; a fresh read still sees 31.
- The last checks write 3 and 5 to key 10, then use fresh reads as bounds of two
  different array aliases. Both types have the expected sizes.

Do not advertise this as a general global-variable replacement. Same-line calls,
line collisions across files, repeated call sites, extra wrappers, and evaluation
order can alter the behavior. Loops would need fresh invocation identities,
and fresh identities alone do not solve call-depth or enclosing-wrapper caching.
The assertions establish the listed top-level evaluation pattern. There is no
cross-translation-unit or cross-build persistence claim.

## Verification

Selected source SHA-256:

```text
4b3396a997e587666a73a720b0f042a461c54f35b155146e9707c59c7d8fc1f3
```

| Source / environment | Flags | Outcome |
| --- | --- | --- |
| Full selected source, local Ubuntu GCC 13.3.0 | `-std=c++23 -O0 -fmax-errors=3` | Pass |
| Same, local GCC 13.3.0 | `-std=c++23 -O2 -fmax-errors=3` | Pass |
| Same, CE x86-64 GCC 16.2 (`g162`) | `-std=c++23 -O0 -fmax-errors=3` | Pass; no diagnostics |
| Same, CE GCC 16.2 | `-std=c++23 -O2 -fmax-errors=3` | Pass; no diagnostics |
| Same, CE Clang 22.1.0 (`clang2210`) | `-std=c++23 -O0 -ferror-limit=1` | Fails first post-write read: 0 instead of 42 |
| Same, CE Clang 22.1.0 | `-std=c++23 -O2 -ferror-limit=1` | Same failure |
| Same, CE GCC 16.2 | `-std=c++23 -O2 -fconstexpr-cache-depth=0 -fmax-errors=1` | Same failure |

Checks cover absent keys; overwritten values; a negative key and key zero;
a full-width mixed pattern, high bit alone, all bits, and zero; interleaved keys;
stale misses; replayed writes; and type formation from later reads. GCC's exact
local package version was `Ubuntu 13.3.0-6ubuntu2~24.04`.

The following bounded control isolates the primitive. Append it to just the
`ember` definition, without the dictionary functions or checks:

```cpp
constexpr int before = __builtin_constant_p(ember(7, 0, 0));
static_assert(ember(7, 0, 0, 500) == 0);
constexpr int after = __builtin_constant_p(ember(7, 0, 0));
static_assert(before == BEFORE);
static_assert(after == AFTER);
```

All four CE runs below passed at `-std=c++23 -O2`, with `-DBEFORE=...` and
`-DAFTER=...` selecting the expected values:

| Compiler / additional setting | Before | After |
| --- | ---: | ---: |
| GCC 16.2, defaults | 0 | 1 |
| GCC 16.2, `-fconstexpr-cache-depth=0` | 0 | 0 |
| GCC 16.2, `-fconstexpr-depth=1024` | 1 | 1 |
| Clang 22.1.0, default evaluator | 0 | 0 |

**Do not run the full dictionary under the raised-depth control.** Every cold
revision marker can then appear present, so the unbounded scan's premise fails.
The bounded control is enough to demonstrate that failure mechanism. The tested
default recursion depth is 512 and cache depth is 8; see
[GCC's dialect options](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html).
The 500/520 constants leave only a small allowance for extra evaluator frames.

CE compiler IDs were freshly discovered from `/api/compilers/c++`. HTTP responses
were checked separately from compiler exit codes, and retained scratch responses
include source text, source hash, flags, and diagnostics. As in the earlier
handoff, those raw records are not committed as an archive. No new public Godbolt
short link was created. The root source and bounded control above are sufficient
to reproduce these claims.

## Provenance and next directions

This was developed by Codex during Tor's continuation of the project. It is an
application of the existing cache counter: bit-addressed membership plus a
per-key append-only revision history. A limited web search found no exact match;
this is not a historical novelty claim. The earlier counter's related-work
distinctions still apply.

## The singleton vector

While this work was underway, Tor suggested that implementing a global singleton
vector or map would be hilarious. Codex added
[cache-vector.cpp](../../tricks/gcc-memoization/vector/vector.cpp) as a second standalone spell. It repeats
the small cache-memory primitive so it can be compiled independently.

`inline constexpr vector v` is an empty global handle. All its methods are const;
its cache-backed contents nevertheless change across constant evaluations. Key
−1 stores length; nonnegative keys store elements. The API is `push_back`,
`pop_back`, `clear`, `size`, and read/write `operator[]`. The sketch uses unsigned
64-bit values and int indices; it is vector-shaped, not a `std::vector` replacement.
It has no allocation, capacity, iterator, or contiguous-data interface.

An index proxy stores two ordinary integers, the index and caller line. The
subscript operator's default second argument is `__builtin_LINE()`, accepted
by both tested GCC versions in C++23 mode. Thus `v[i]` carries a fresh identity
at distinct source lines, without spelling an extra argument. The proxy forwards
conversions to `recall(index, line)` and assignment to
`remember(index, value, -line)`.

Before changing the length, push/pop use `recall(-1, -line)`. Public `size()`
uses `recall(-1, line)`. Opposite signs keep an initial length query distinct from
the post-write query on the same source line in `(v.push_back(42), v.size())`.
This only distinguishes those two phases; it is not a general fresh-call scheme.

The complete source checks the class is empty and size 1, push/read, successive
pushes, element assignment, pop, push into a popped position, clear/reuse,
zero/all-one elements, and a second global handle observing and changing the
same singleton. A saved proxy is read, the element is changed, and the saved
proxy still reads its old value while a new subscript sees the new value. Preserve
this limitation: it is not a normal reference. Pop requires a nonempty vector,
and indexing/assignment requires an index in the live range.

Selected vector source SHA-256:

```text
0db41939b28b8288bda424f19db2048e957264074e33c46bcc483fbc4206618b
```

| Environment | Flags | Outcome |
| --- | --- | --- |
| Local Ubuntu GCC 13.3.0 | `-std=c++23 -O0 -fmax-errors=3` | Pass |
| Same | `-std=c++23 -O2 -fmax-errors=3` | Pass |
| CE GCC 16.2 (`g162`) | `-std=c++23 -O0 -fmax-errors=3` | Pass; no diagnostics |
| Same | `-std=c++23 -O2 -fmax-errors=3` | Pass; no diagnostics |
| CE Clang 22.1.0 (`clang2210`) | `-std=c++23 -O2 -ferror-limit=1` | Fails size check after the first push |

The dictionary's cache controls apply to the identical primitive here; they
were not needlessly rerun on the whole vector. All successful source-hashed
records in the matrix above refer to the complete selected source, including
the second-handle and saved-proxy checks.

## Possible next work

A tiny compile-time register machine or interning table could use the dictionary.
Keep such examples small and explicitly account for fresh call identities and
evaluation depth. Another question is whether structured values passed by value
can supply content-based cache keys without hashing; that was not tested here.
Neither is a task running in the background.
