# Notes on the spells

Developed in a conversation between Tor Shepherd and Codex on 2026-09-07.
Compiler Explorer targets: x86-64 GCC 16.2 (`g162`) and Clang 22.1.0
(`clang2210`). The assertions in the repository sources pass at
`-std=c++23 -O0` and `-std=c++23 -O2` on the compilers listed in the README.
Cache memory and cache vector were also checked locally with Ubuntu GCC 13.3.0.

## One-pointer vector

[Source](one-pointer-vector.cpp) · [Godbolt sketch](https://godbolt.org/z/MscKzxdjf)

Each slot is a union of an empty marker and a value. The evaluator tracks which
member is active, so the vector itself stores just one pointer.

The compact version probes the `char empty` member, rather than trying to
recognize an arbitrary `T` as constant. This also works for the tested structs
and pointers. A reserved empty slot always follows the occupied prefix:
`size()` scans to that slot; `capacity()` continues through empty slots until
the allocation ends, then subtracts the reserved slot.

Growing from size `n` allocates `2 * n + 2` slots, of which `2 * n + 1` are
usable. Capacity therefore grows through 1, 3, 7, ... . The final empty slot is
an implementation sentinel; no value of `T` is reserved.

Assigning a new union with an active value fills a slot. Activating `empty`
pops it. The compiler supplies the missing tags, bounds, and lifetimes.

The sketch expects trivially copyable/destructible element types suitable for
these union operations. Checks cover integers, doubles, booleans, small
aggregates, pointers (including null), and a class with a converting constructor.
It does not attempt general container semantics: keep the owning vector
uncopied, pop only when nonempty, and index live elements. Queries scan linearly.
Allocation and deletion happen within the same constant evaluation.

The Godbolt sketch uses C++20 and a shorter demonstration. The repository keeps
the fuller checks and has been checked in C++23.

## Empty bits

[Source](empty-bits.cpp) · [Godbolt](https://godbolt.org/z/cxqejscT1)

The class inherits 64 distinct empty base types. On the tested target it
satisfies both `sizeof(integer) == 1` and `std::is_empty_v<integer>`.

Each base's lifetime represents a bit. Destroying the base clears the bit;
constructing it sets the bit. Clang's constant evaluator distinguishes a live
base from a dead one when `__builtin_constant_p` probes a non-static member call,
even though that member simply returns true.

The payload is supplied and changed through ordinary function arguments during
constant evaluation. Two objects of the same type can hold different values.
The template arguments identify bit positions, not the stored number.

The destructor revives the bases before their implicit destruction. The state
belongs to evaluator lifetime bookkeeping, so copying the wrapper's bytes does
not copy that state; keep the wrapper uncopied and use it only in constant
evaluation. This relies on Clang's handling of empty-base lifetime replacement.
It is an observed compiler-extension result, not a portability claim or a way
to compress runtime integers.

GCC 16.2 rejects this example. The checks on Clang cover two independent objects,
sixteen generated patterns, repeated zero/all-one writes, mutation, and
destruction.

## Cache counter

[Source](cache-counter.cpp) · [Godbolt](https://godbolt.org/z/j6d7b6nEn)

An uncached `slow(key, 520)` exceeds the tested default recursion budget.
`__builtin_constant_p` converts that failure into zero. Evaluating
`slow(key, 500)` fits and seeds GCC's constexpr memoization cache; a later
520-level probe can then finish through cached subcalls.

Warm keys form an observable set. `next()` scans from zero to the first cold
key, warms it, and returns it. No C++ global object holds the count.

The unused default argument gives `next()` itself a different cache key on
different source lines. It does not supply the count. Reusing an explicit
argument demonstrates that the wrapper is memoized too: 4, 4, then 5.
Calls on the same line, repeated execution of one call site, and line-number
collisions across files can reuse that key.

[The smaller cache probe](https://godbolt.org/z/v3MT7f8xG) asks whether
`f(520)` is constant before and after evaluating `f(500)`:

| Compiler and additional options | Before | After |
| --- | ---: | ---: |
| GCC 16.2 | 0 | 1 |
| GCC 16.2, `-fconstexpr-cache-depth=0` | 0 | 0 |
| GCC 16.2, `-fconstexpr-depth=1024` | 1 | 1 |
| Clang 22.1.0 | 0 | 0 |

These controls were run at `-std=c++23 -O2`. GCC documents
[its recursion and cache controls](https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html).
The constants depend on those limits. Disabling caching or increasing the
recursion budget breaks the counter's mechanism; Clang does not pass the
counter's increment assertions.

## Cache memory

[Source](cache-memory.cpp)

The cache counter can remember membership. Give each cached fact a key, a
revision, and a bit position, and it can remember whole values:

```cpp
static_assert(remember(7, 42) == 42);
static_assert(recall(7) == 42);
static_assert(remember(7, 99) == 99);
static_assert(recall(7) == 99);
```

These are separate constant evaluations. No C++ object carries the dictionary
between them; there are no templates, friend injections, or mutable globals in
the spell. Keys and values arrive as ordinary function arguments. The compiler
does spend memory retaining its cache.

`ember(key, revision, bit, 500)` warms one fact. The corresponding 520-deep
probe recognizes a warm fact and fails on a cold one under the tested default
limits. Positions 0–63 encode the set bits of an unsigned 64-bit value.
Position 64 marks a completed revision, including a write of zero.

`remember` finds the first unmarked revision, warms its set bits, then marks it.
`recall` scans the same markers and reconstructs the last completed value.
An unwritten key reads as zero. Overwriting never clears cache entries: the new
revision supersedes the old one. The revision scan grows linearly with the
number of writes to that key. There is no erase or reclamation.

The cache also remembers calls to `remember` and `recall` themselves. Their
unused line-number arguments distinguish the top-level calls in this sketch.
Reusing an identical read tuple can return a stale value; replaying an identical
write tuple can return its old result without writing again. The checks exhibit
both, including a cached miss surviving a later write. Calls from a loop or
wrapper need their own investigation: a source line is not a fresh invocation
ID, and wrapper memoization and extra call depth also matter.

The final checks use successive reads as array bounds: the same key supplies
3 and then 5 to two type aliases. That demonstrates state influencing later
type formation; it does not make an arbitrary function parameter a constant
expression within that function's definition.

The complete source passes GCC 13.3.0 and GCC 16.2 at C++23 `-O0` and `-O2`.
Clang 22.1.0 fails at both levels, reading zero after the first write. GCC 16.2
with caching disabled also fails that read. A bounded primitive control gives
the same 0→1 / 0→0 / 1→1 results as the counter's controls above.

Keep the default constexpr depth and cache settings. Raising the recursion
limit enough to make cold facts recognizable breaks the revision scan; do not
try that control on the unbounded full spell. This is observed evaluator
behavior within one translation unit, not ISO-portable storage or persistence
across compilations. The constants leave only a small margin for extra call
depth. [Working notes](working-notes/06-cache-memory.md) preserve the controls
and exact source hash. This is an application of the existing cache counter,
without a claim of historical priority.

## Cache vector

[Source](cache-vector.cpp)

An empty class provides a vector-shaped interface to the same cache memory.
Its global handle is const, yet later constant evaluations observe pushes,
pops, and element writes:

```cpp
inline constexpr vector v;
static_assert(v.size() == 0);
static_assert((v.push_back(42), v.size()) == 1);
static_assert(v[0] == 42);
static_assert((v[0] = 7) == 7);
static_assert(v[0] == 7);
```

Cache key −1 holds the length; nonnegative keys hold elements. Popping or
clearing updates the length, leaving old element histories in the cache. A push
overwrites the next logical position. Every handle addresses this same singleton,
as the checks demonstrate with a second empty global object.

The `reference` proxy holds an index and a call identity, and forwards reads and
assignments to the cache. `operator[]` has a hidden default line-number argument,
so ordinary `v[i]` syntax supplies that identity. The
[subscript rules](https://eel.is/c++draft/over.sub) allow default arguments, and
[GCC documents](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
the caller-line behavior of `__builtin_LINE()` in default arguments. Both tested
GCC versions accept this in C++23 mode.

Mutation operations use the negative caller line for their initial length read;
public `size()` uses the positive line. This separates the pre-write and
post-write lookups in `(v.push_back(42), v.size())`. It does not make arbitrary
same-line sequences or repeated call sites work. A saved proxy also saves its
identity: after it has been read, a later read can remain stale despite an element
write. The final assertions show this explicitly.

The handle has no data members and has size 1 on the tested targets. The proxy
has two ordinary integers; no claim of an empty proxy is made. This is a sketch
with `unsigned long long` elements and `int` indices, not an implementation of
`std::vector`. Index only live elements and pop only when nonempty. It supplies
no contiguous storage, capacity, iterator, or general reference semantics. The
cache memory's compiler, call-depth, cache-key, and resource-limit restrictions
all still apply.

The complete standalone file passes GCC 13.3.0 and GCC 16.2 at C++23 `-O0` and
`-O2`. Clang 22.1.0 at `-O2` fails the size check after the first push. See the
[session notes](working-notes/06-cache-memory.md#the-singleton-vector) for its
source hash and verification details.

## Typed cache experiments

The cache's 64-bit value restriction is a choice of encoding. Follow-up
[typed experiments](working-notes/07-typed-cache.md) passed on GCC 16.2 with
double, enum, and 24-byte struct values, and with an owning `std::string`
serialized as length and characters. The selected root dictionary/vector still
use unsigned 64-bit values. The broader sketches require values that can be
encoded and reconstructed in constant evaluation; raw byte encoding does not
automatically handle padding, pointers, or arbitrary object identity.

## Related work and provenance

The constructions were developed and tested during the conversation. A targeted
prior-art search found no exact match for the cache counter or the empty-base
bit store; this is not an established claim of first discovery.

[Filip Roséen's 2015 constexpr counter](https://refp.se/articles/constexpr-counter)
uses friend injection. It is related stateful metaprogramming with a different
storage mechanism.

[P2641R4, section 3.5](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2641r4.html#implementation-experience)
already documents using `__builtin_constant_p` for lifetime/active-union
detection, crediting Johel Ernesto Guerrero Peña and Ed Catmur. That ingredient,
including the active-union probe used by the vector, is known prior work.

[GCC's builtin documentation](https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html)
describes constant recognition, not a universal dereferenceability test.
The probes in these spells are deliberately narrow; arbitrary expressions,
especially ones with side effects, need their own investigation.
