# Notes on the spells

Developed in a conversation between Tor Shepherd and Codex on 2026-09-07.
Compiler Explorer targets: x86-64 GCC 16.2 (`g162`) and Clang 22.1.0
(`clang2210`). The assertions in the root spell sources pass at
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

## Sortable cache vector

[Advanced source](advanced/cache-vector.cpp) ·
[Research handoff](working-notes/08-sortable-cache-vector.md)

The simple integer dictionary and vector remain unchanged. This separate
application adds serialization and standard algorithms without pretending they
are free additions to the original spell.

`cache_vector<T, Codec, Tag>` stores a singleton per specialization. `Codec`
supplies `encode(const T&) -> bytes` and `decode(bytes) -> T`, both usable in
constant evaluation; `bytes` is a vector of unsigned characters. The default
codec bit-casts suitable object representations. The example text codec stores
characters, and a field-wise codec handles a padded, non-default-constructible
record. Neither pointer identity nor arbitrary object representations are solved.

Each revision has a completion marker. Each encoded byte has eight bit facts
and a presence marker, so a read can discover the encoded length, including zero.
The logical vector length uses key −1. No codec is asked to manufacture a missing
element: an unwritten element read throws, while an absent length starts at zero.

### Fresh calls inside algorithms

The old line-number argument is not enough for repeated accesses from a loop.
Here each cache read/write also receives the address of a local handle. That
parameter is intentionally unused by the body. In the tested GCC, the wrapper
calls stay fresh while the integer-only `ember` calls still retain their facts.
A reduced local-address loop passes; replacing the address with null fails.
This supports the cache-eligibility explanation, not a portable semantic rule.

Use a local handle inside the enclosing constant evaluation:

```cpp
static_assert([] {
    auto v = numbers;
    std::ranges::sort(v);
    return std::ranges::is_sorted(v);
}());
```

The handle is empty and copies no elements. Another evaluation's local handle
sees the same singleton contents. Iterators and proxies carry its address, so
they must not outlive it. Unlike the simple spell's saved proxy, a saved proxy
in this access pattern observes later writes; the checks demonstrate that.
Do not assume calls through a global handle or memoized enclosing function are
fresh just because this local-handle pattern is.

### Sorting and swapping

The iterator satisfies the tested `random_access_iterator` and `sortable`
constraints for integers and strings. The checks exercise `std::ranges::sort`,
`std::sort`, both sortedness predicates, and exact sequence comparisons. A
20-element case goes beyond libstdc++'s small-range insertion-sort path.

Proxy assignment writes a decoded value, not a new index. ADL `swap` materializes
one value before the writes; iterator `iter_move` and `iter_swap` customize the
ranges operations. The checked swap interfaces are:

```cpp
using std::swap;
swap(v[0], v[1]);
std::iter_swap(v.begin(), v.begin() + 1);
std::ranges::swap(v[0], v[1]);
std::ranges::iter_swap(v.begin(), v.begin() + 1);
```

This is not an overload added to `std`. Use ADL or the ranges customization
points, not explicitly qualified `std::swap` on proxy objects: copying a proxy
copies its location, not a snapshot of the element.

### Limits and reproduction

The advanced source passed locally on Ubuntu GCC 13.3.0 with:

```sh
g++ -std=c++23 -O2 -fconstexpr-cache-depth=64 -c advanced/cache-vector.cpp -o /tmp/sortable-cache.o
```

`-O0` also passes. Keep the default constexpr recursion limit of 512. The
advanced spell warms at depth 384 and probes at 640, leaving room for algorithm
wrappers; cache depth 64 permits writes from those deeper calls to be retained.
The initial integer-sort prototype failed with the default cache depth 8 and
passed with 64. No claim of GCC 16.2 or Clang verification is made for this file.

Index only live elements, pop only when nonempty, and keep indices and encoded
sizes within the sketch's `int` arithmetic. Structural changes require fresh
range endpoints. The iterator is not contiguous and the proxy is not `T&`.
Codecs must round-trip values; sorting also needs the usual ordering and value
operations. All reconstructed allocations must be destroyed within their
constant evaluation. Cache history is never reclaimed, and revision scans and
bit probes are expensive. This is compiler behavior as art, not a runtime or
ISO-portable container implementation.

## Copy gate

[Source](copy-gate.cpp) · [Research handoff](working-notes/09-copy-gate.md)

A function declaration accepts a word only when its characters are already
sorted. Its signature has no constraint, and neither overload needs a body:

```cpp
template<letters S> std::true_type sorted(word<S>);
std::false_type sorted(...);

static_assert(decltype(sorted(word<"abc">{}))::value);
static_assert(!decltype(sorted(word<"cab">{}))::value);
```

`letters` preserves the input in its string constructor and sorts it in its copy
constructor. On the tested GCC, deduction from `word<"cab">` followed by
substitution reconstructs the candidate's parameter type as `word<"abc">`.
The original argument cannot convert to that type, so the ellipsis wins.
For `"abc"`, the copy leaves the value unchanged and the exact-match overload
wins. Removing the fallback exposes a diagnostic naming both spellings;
removing sorting from the copy constructor admits both inputs.

This makes overload viability a test for fixed points of copying. No sortedness
predicate is called: the copy performs normalization, and type matching detects
whether that normalization changed anything. The calls occur only in `decltype`;
the work happens while constructing template arguments and resolving the call.
The last array element is treated as the string terminator and excluded from
sorting. Embedded nulls before it participate normally.

The equality involved is **template-argument equivalence**. For class values,
the compiler recursively compares corresponding subobjects, including each
array element, to determine whether two template arguments name the same
specialization. This is a language rule, not a call to `operator==` and not a
raw byte comparison. See [C++23's type-equivalence rules](https://timsong-cpp.github.io/cppwp/n4950/temp.type#2).

`letters` has no equality operator in the original spell. Separate controls
with `operator==` deleted, always true, and always false all retained the same
sorting-gate results on GCC 13.3 at C++23 `-O0`. Even an operator claiming that
`letters{"cab"} == letters{"abc"}` cannot make `word<"cab">` and `word<"abc">`
the same type. Copying changes the candidate's template argument; the compiler's
structural equivalence rule then determines that the parameter type no longer
matches. The [working notes](working-notes/09-copy-gate.md#equality-controls)
record the exact controls.

The associated [copy-driven factorial](working-notes/09-copy-gate.md#copy-driven-return-type-computation)
uses several techniques: `if constexpr` supplies decisions, ordinary constexpr
code reads and updates state, and return-type deduction drives recursive
instantiation. It does not establish a roughly-Turing machine using hidden
copying alone. Tor's [Pedantics](working-notes/Pedantics.md) requires each
internal operation to stay within an explicitly chosen set of techniques.
“Hidden copy plus overload resolution” is a possible set to investigate;
recursion within that set has not been demonstrated. The
[corrected assessment](working-notes/09-copy-gate.md#roughly-turing-assessment)
records this limit without changing the observed compiler results.

The complete source passes local Ubuntu GCC 13.3.0 with C++23, `-Wall -Wextra
-pedantic-errors`, and both `-O0` and `-O2`. It also passes with constexpr caching
disabled, independently of the cache-storage spells. No Clang or newer-GCC
result is claimed.

**This is not an ISO-portability claim.** Jelle Hellings documented differing
compiler treatment of changing copy constructors and template forwarding in
[December 2022](https://jhellings.nl/article?articleid=1). That is prior art for
the ingredient; this session assembled the sorting gate and its controls.
[P2308R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2308r1.html),
adopted in November 2023 to resolve
[CWG2459](https://cplusplus.github.io/CWG/issues/2459.html), requires the copy
that initializes a template parameter object to preserve template-argument
equivalence. Under that wording, the changing-copy unsorted argument is itself
ill-formed; a conforming implementation need not reach our fallback. GCC 13.3's
acceptance, even with pedantic diagnostics enabled, does not settle conformance.

## Last rites

[Source](last-rites.cpp) · [Editable Godbolt](https://godbolt.org/z/jaoEYe4Y4) ·
[Research handoff](working-notes/11-last-rites.md)

The semicolon differentiates the expression:

```cpp
static_assert([] {
    number x{3}, y{4};
    double result = (x*x + x*y).backward();
    return result == 21 && x.gradient == 10 && y.gradient == 3;
}());
```

Each arithmetic operator returns a temporary containing its value, pointers to
its operands, and two local derivatives. `backward()` only sets the root's
gradient to one and returns its value. The destructor adds its gradient times
each local derivative to the corresponding operand's gradient.

Operands are constructed before the result that depends on them. At the end of
the full expression, temporary destruction supplies the reverse traversal:
each result propagates before its temporary operands are destroyed. The
[C++23 temporary rules](https://timsong-cpp.github.io/cppwp/n4950/class.temporary#8)
provide that ordering. Sibling evaluation order can vary; either order respects
these dependencies. As usual, floating-point accumulation can depend on order.

There is no separately allocated tape, explicit traversal, or recursive
backward call. The temporary objects still occupy storage proportional to the
expression. This is numerical reverse-mode differentiation using ordinary
arithmetic and `constexpr` destructors; it does not produce a symbolic formula.
No templates or compiler builtins appear in the spell.

Keep operation nodes as temporaries within one full expression, with input
variables surviving it. Read gradients in the next statement. Saving a nested
expression in a named variable can leave it pointing at already-destroyed
temporaries. Copies and assignments are deleted, but direct initialization can
still create that invalid lifetime arrangement. Each expression's graph is
consumed once; input gradients accumulate across successive expressions.
The small spell implements `+` and `*` and assumes finite arithmetic throughout.

All eight assertions pass on GCC 13.3 at `-O0` and `-O2`, and on Godbolt GCC
16.2 at `-O2`, with C++23 and pedantic diagnostics. The GCC 13.3 O2 check also
disables constexpr caching and optional copy elision. Clang 22.1.0's default
evaluator rejects the nested cases with a lifetime diagnostic; its experimental
new evaluator also fails assertions. The reduced disagreement and runtime
sanitizer check are recorded in the handoff. No Clang portability claim is made.

Reverse-mode differentiation and operator-overloading AD are established work;
the [Stan Math paper](https://arxiv.org/abs/1509.07164) describes an explicit
reverse traversal of recorded nodes. This session independently assembled the
temporary-destructor scheduling variant. The limited prior-art search did not
establish historical novelty.

## False idols

[Source](false-idols.cpp) · [Proof and verification](working-notes/12-false-idols.md)

Overload resolution decides satisfiability of a Boolean formula even when every
atomic constraint is literally `true`. There are no function bodies or recursive
solving helpers in the C++ source.

Each positive and negative literal gets its own named concept. `X` and `NX` both
evaluate to true; `NX` represents a negative literal and is not C++ `!X`. A second
formula describes inconsistent selections:

```cpp
template<class T> concept contradiction =
    (X<T> && NX<T>) || (Y<T> && NY<T>);
```

The original SAT problem has no solution exactly when its encoded formula
implies this contradiction formula. C++ preserves named atomic identities and
compares AND/OR structure during constraint subsumption. For these monotone
formulas, its ordering test realizes that implication query.
The [standard's subsumption rule](https://eel.is/c++draft/temp.constr.order)
and [atomic identity rules](https://eel.is/c++draft/temp.constr.atomic) are the
ingredients; the reduction and proof are in the research notes.

```cpp
template<class T> struct oracle {
    void solve() requires contradiction<T>;
    void solve() requires (formula<T> && true);
};
template<class T> concept satisfiable =
    !requires(oracle<T> o) { o.solve(); };
```

Both overloads are viable. For an unsatisfiable problem the second is more
constrained; for a satisfiable one they are incomparable and the call is
ambiguous. The final `true` contributes a fresh atomic identity, preventing
reverse subsumption and equal-formula ties. The `requires` expression observes
the final answer without calling either function.

The standalone file includes UNSAT and SAT cases. It passes GCC 13.3, GCC 16.2,
and Clang 22.1.0 in C++20. The newer compilers also pass C++23 at `-O2` and C++20
at `-O0` with constexpr depth limited to one. An independent generated suite
checks 389 cases on all three compilers. Hiding the formula behind an ordinary
Boolean preserves its value but destroys the structural UNSAT proof.

Inputs are source-level constraint expressions. The spell does not extract a
satisfying assignment, and finite SAT solving is not an isolated roughly-Turing
machine. Normalization can grow exponentially; this is not a practical SAT
solver or a performance claim. Corentin Jabot discusses that existing cost in
[his Clang implementation account](https://cor3ntin.github.io/posts/clang21/#faster-subsumption).
The all-true encoding and ambiguity oracle were independently assembled in this
project; the targeted prior-art search does not establish historical priority.

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
