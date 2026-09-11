# Out-of-left-field search after Astral heap

September 9, 2026. Tor requested a genuinely new standalone technique after
Astral heap, not another extension of an existing mechanism. This note preserves
the broad search, concrete probes, rejected candidates, and the one promising
unfinished result. No new reader-facing spell was selected in this session.

September 10–11 follow-up: [Phase shift](../../tricks/phase-shift/) now has a
standalone source, walkthrough, cross-compiler main-source evidence, and retained
controls. [Note 19](19-phase-shift.md) records that bounded continuation and
corrections to this note's `typeid` transcription and trinary evidence.

## The bar and exclusions

Tor restated the two useful targets:

- a capability that sounds intrinsically absurd, like a zero-size integer; or
- a narrowly isolated language feature that unexpectedly supplies the
  ingredients of a roughly-Turing machine.

The search treated the following families as excluded rather than convenient
ingredients: GCC constexpr memoization, active-union or subobject-lifetime
storage, pointer provenance/bounds/allocation identities, secret copy/equality,
destructor scheduling or gradients, constraint-subsumption SAT, empty-base
layout/coloring, Seance instantiation ghosts, and Astral heap's sparse evaluator
allocations. Ordinary algorithms merely executed during constant evaluation did
not qualify.

The initial mechanism map separated compiler/evaluator metadata already used in
the repository: call-stack headroom, memoization, active members and lifetimes,
allocation identity and extent, sparse array values, pointer provenance,
constant-knownness, delayed instantiation, and object layout. A new spelling for
one of those channels was not enough.

## Concrete probes

### Recursive `operator->`: one arrow as a fixed loop

C++ recursively reapplies `operator->` until it obtains a pointer. A local C++23
probe encoded a countdown in proxy types. Evaluating one source-level
`a->value` called 101 user-defined `operator->` functions, incremented a counter
at each step, and reached the final pointee. GCC 13.3 accepted its assertion at
the checked default settings.

This is a wonderfully odd language rule, but not a selected discovery. The
countdown transition used template arithmetic, termination used a class-template
specialization, and the state lived in ordinary types and an ordinary integer.
The arrow supplied repetition, not the whole computation. Recursive arrow
processing is also established and publicly explained; this prototype did not
add a new primitive or meet Tor's isolated-machine rules.

The passing probe was:

```cpp
struct target { int value = 42; };

template<int N>
struct arrow {
    int* count;
    constexpr auto operator->() const {
        ++*count;
        return arrow<N - 1>{count};
    }
};

template<>
struct arrow<0> {
    int* count;
    target object{};
    constexpr auto operator->() {
        ++*count;
        return &object;
    }
};

constexpr int test() {
    int count = 0;
    arrow<100> a{&count};
    return a->value + count;
}

static_assert(test() == 143);
```

### Aggregate appertainment: braces descend a type tree

Aggregate initialization's appertainment algorithm repeatedly replaces an
aggregate element with its subelements until an initializer clause appertains.
A local C++23 probe nested four aggregate levels and supplied an object convertible
to one intermediate level. GCC 13.3 accepted assertions showing that the clause
initialized that level while outer default member initializers remained intact.

This makes braces look like a compiler-driven packet router, but the traversal
has an important boundary: it cannot skip an earlier scalar leaf that the clause
cannot initialize. The mechanism chooses a depth along the current leftmost
descent rather than performing a general search among arbitrary sibling branches.
No stronger state, solver, or isolated machine was demonstrated, so this remained
an unselected curiosity.

The passing depth-routing probe was:

```cpp
struct bottom { int value; };
struct level3 { bottom child; int marker = 3; };
struct level2 { level3 child; int marker = 2; };
struct level1 { level2 child; int marker = 1; };
struct root { level1 child; int marker = 0; };

struct to_level2 {
    int value;
    constexpr operator level2() const {
        return {{{value}, 30}, 20};
    }
};

constexpr root routed{to_level2{42}};
static_assert(routed.child.child.child.child.value == 42);
static_assert(routed.child.child.marker == 20);
static_assert(routed.child.marker == 1);
```

### `typeid`: a return type decides whether a call happens

The operand of `typeid` is evaluated when it is a glvalue of polymorphic class
type and otherwise is unevaluated. A local constexpr probe put `++n` in the
operand and changed only the operand's class type. GCC 13.3 accepted assertions
that `n` remained zero for a non-polymorphic type and became one for a
polymorphic type.

The sharper phrasing is that a function returning `T&` can be called or not
called solely according to whether `T` is polymorphic. This provides a static
conditional side effect with no `if`, conditional operator, trait, or constraint.
It is genuinely orthogonal to the repository's evaluator-state mechanisms, but
it is a direct, long-documented `typeid` rule rather than a sufficiently strong
new construction. It was not selected.

The local probe used this essential function:

```cpp
#include <typeinfo>

struct plain {};
struct poly { virtual constexpr ~poly() = default; };

template<class T>
constexpr int probe() {
    int n = 0;
    T object{};
    (void)typeid((++n, static_cast<T&>(object)));
    return n;
}

static_assert(probe<plain>() == 0);
static_assert(probe<poly>() == 1);
```

The function-returning-`T&` phrasing was reasoned from the same rule but was not
separately compiled in this session.

Correction: an earlier transcription used `T{}` instead of the object reference
above. That prvalue does not provide the evaluated polymorphic-glvalue case and
does not justify the second assertion. The distinction is essential, not just
a syntactic reduction. See the [`typeid` rule](https://eel.is/c++draft/expr.typeid).

### C++26 constexpr exceptions

A minimal `consteval` function threw `42`, caught it, and returned the caught
value. A Compiler Explorer request used `-std=c++2c -Wall -Wextra
-pedantic-errors`:

| Compiler target | Result |
| --- | --- |
| GCC 16.2 (`g162`) | Accepted |
| Clang 22.1.0 (`clang2210`) | Rejected the throw in a constant expression |
| Clang trunk (`clang_trunk`) | Rejected likewise |
| attempted GCC ID `g0` | HTTP 404; it was not a discovered valid target |

The tested source was:

```cpp
consteval int caught() {
    try {
        throw 42;
    } catch (int answer) {
        return answer;
    }
}

static_assert(caught() == 42);
```

Ideas considered included a `void` function delivering a compile-time result by
throwing it, handler matching as type selection, and rethrow/unwinding as control
flow. They remained ordinary exception programs with normal function recursion
or other helpers doing the computation. No exception-only recurrence mechanism
or isolated machine was demonstrated. Compiler support was also split. This path
was not developed further.

### The rejected initializedness family

A local GCC probe used `__builtin_constant_p` to distinguish an uninitialized
`unsigned char` from each initialized byte value without evaluating the invalid
read arm of a conditional expression. GCC 13.3 accepted C++20 and C++23 checks
for the extra sentinel state and for a transition from uninitialized to an
ordinary byte value.

The idea was then extended to eight one-bit fields: each field can be
uninitialized, zero, or one, giving eight trits and `3^8 = 6561` logical states.
The first concrete prototype used `unsigned` bit-fields and did **not** establish
the one-byte result: local GCC gave the class size four, and the hand-calculated
mixed ternary assertion was also wrong.

The corrected `unsigned char`-bit-field probe was completed before the
same-mechanism rejection. GCC 13.3 accepted
all of the following with C++20 and C++23, O0 and O2, warnings, and pedantic
errors enabled:

- `sizeof(trinary_byte) == 1`;
- eight untouched fields read logically as `22222222` base three, or 6560;
- assigning `b0 = 0` and `b1 = 1` produces `22222210` base three, or 6555.

Thus the **eight-trits-in-one-byte result is a real finding**. The
unknown state can be changed to zero or one, and an initialized field can change
between zero and one. No operation to restore one field independently to the
unknown state was demonstrated. Source-specific Compiler Explorer requests also
reported acceptance on GCC 16.2 (`g162`) and Clang 22.1.0 (`clang2210`) with
`-std=c++23 -O2 -Wall -Wextra -pedantic-errors`.

Evidence correction: the first documentation pass incorrectly called the
trinary request stale and omitted those remote verdicts. Reviewing the session
record showed that its request had been rebuilt from the corrected source.
The conceptual rejection below is unaffected by this correction.

The corrected source was:

```cpp
struct trinary_byte {
    unsigned char b0 : 1;
    unsigned char b1 : 1;
    unsigned char b2 : 1;
    unsigned char b3 : 1;
    unsigned char b4 : 1;
    unsigned char b5 : 1;
    unsigned char b6 : 1;
    unsigned char b7 : 1;

    constexpr trinary_byte() {}

    constexpr unsigned digit(unsigned i) const {
        switch (i) {
        case 0: return __builtin_constant_p(b0) ? b0 : 2;
        case 1: return __builtin_constant_p(b1) ? b1 : 2;
        case 2: return __builtin_constant_p(b2) ? b2 : 2;
        case 3: return __builtin_constant_p(b3) ? b3 : 2;
        case 4: return __builtin_constant_p(b4) ? b4 : 2;
        case 5: return __builtin_constant_p(b5) ? b5 : 2;
        case 6: return __builtin_constant_p(b6) ? b6 : 2;
        default: return __builtin_constant_p(b7) ? b7 : 2;
        }
    }

    constexpr unsigned get() const {
        unsigned n = 0;
        unsigned place = 1;
        for (unsigned i = 0; i != 8; ++i, place *= 3)
            n += digit(i) * place;
        return n;
    }
};

static_assert(sizeof(trinary_byte) == 1);
static_assert(trinary_byte{}.get() == 6560);

constexpr unsigned pattern() {
    trinary_byte x;
    x.b0 = 0;
    x.b1 = 1;
    return x.get();
}

static_assert(pattern() == 6555);
```

Tor's rejection is preserved verbatim:

> Isn't this quite similar to our existing zero-byte integer trick? It's the same mechanism, no?

The response and decision are also preserved verbatim:

> Yes—good catch. It’s not literally the same bit (the existing integer stores state in subobject lifetimes; this stores a third state in bit-field initializedness), but it is absolutely the same underlying species: evaluator bookkeeping + `__builtin_constant_p` used as invisible storage. “Eight trits in a byte” is cute, but it fails your completely-new-mechanism constraint.
>
> I’m discarding it, not adding it to the repo. The search needs to leave evaluator liveness/knownness/provenance entirely, not merely find a new flavor of invisible state.

This family is closed for the purpose of the fresh search. Do not revive it as a
new spell merely by correcting the field type, packing, or ternary arithmetic.
It remains preserved here as a valid finding and possible application of the
existing evaluator-metadata family. Tor's later workflow correction is verbatim:

> What I was going to say btw: I saw you say you would discard that trot trick you located. Please don’t discard - write down all of your findings in the working notes in the repo as you go

Here “discarded” means rejected as the unrelated headline technique, never
deleted from research history.

## The implicit-object-creation almost-direction

The most startling language wording encountered outside constant evaluation was
C++'s implicit object creation rule. For an operation specified to implicitly
create objects, the abstract machine creates zero or more implicit-lifetime
objects if some such set gives the program defined behavior. If several sets do,
which set is created is unspecified; if none do, behavior is undefined. P0593R6
describes multiple valid object structures as a superposition.

This sounds like an existential solver: raw storage acquires whatever objects
rescue the later program. It did not become a usable spell here:

- the current object-model wording explicitly excludes the relevant byte-array
  and `operator new` implicit creation during constant evaluation;
- P0593R6 deliberately retained that conservative constant-evaluation boundary;
- runtime undefinedness when no suitable object set exists is not a safe,
  catchable Boolean oracle; and
- `std::bit_cast` does implicitly create objects nested in its explicit result,
  but no construction was found in which the existential choice produced a
  safe observable result rather than ordinary bit casting or undefined behavior.

Primary references examined:

- [Object model](https://eel.is/c++draft/basic.memobj), especially the implicit-
  creation rules and the constant-evaluation exclusions.
- [P0593R6](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0593r6.html),
  especially its constant-expression and `vector` superposition discussions.
- [`std::bit_cast`](https://eel.is/c++draft/bit.cast), which explicitly creates
  objects nested within its result.
- [P2747R2](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p2747r2.html),
  which reiterates that implicit object creation does not happen through the
  named core operations during constant evaluation while adding constexpr
  placement new.

The wording is excellent cursed-C++ material, but this session found no honest
observable technique. Do not turn it into a spell without closing that gap.

## Promising unfinished result: a type changes shape mid-file

Structured bindings use three protocols. For a non-array class `E`, tuple-like
decomposition is selected only if `std::tuple_size<E>` names a **complete** class
with a member named `value`; otherwise the language falls through to direct
member decomposition. An explicit class-template specialization can legally be
declared without being defined and used as an incomplete class.

Those two rules compose into a one-way semantic phase transition:

1. Define a normal two-member `coordinate`.
2. Forward-declare its legal `std::tuple_size<coordinate>` specialization, leaving
   it incomplete.
3. Above the specialization's definition, `auto [x, y] = point;` decomposes the
   two real members.
4. Complete `tuple_size<coordinate>` with value three, provide three
   `tuple_element` specializations, and provide ADL `get<I>`.
5. Below that point, `auto [y, x, sum] = point;` exposes three synthetic elements,
   including a computed sum that is not a member.

The same class definition and object layout remain in force. In the checked
prototype, `sizeof(coordinate) == 2 * sizeof(int)` throughout; the class has two
structured-binding elements earlier in the translation unit and three later.
Earlier function-body semantics remain fixed even when its constexpr function is
called by a later `static_assert`.

The reduced candidate is:

```cpp
#include <cstddef>
#include <tuple>

struct coordinate { int x; int y; };

namespace std {
template<> struct tuple_size<coordinate>; // deliberately incomplete
}

constexpr int before() {
    coordinate point{10, 20};
    auto [x, y] = point;                 // the two real members
    return 100 * x + y;
}

namespace std {
template<>
struct tuple_size<coordinate> : integral_constant<size_t, 3> {};

template<> struct tuple_element<0, coordinate> { using type = int; };
template<> struct tuple_element<1, coordinate> { using type = int; };
template<> struct tuple_element<2, coordinate> { using type = int; };
}

template<std::size_t I>
constexpr int get(coordinate point) {
    int values[]{point.y, point.x, point.x + point.y};
    return values[I];
}

constexpr int after() {
    coordinate point{10, 20};
    auto [y, x, sum] = point;            // three synthetic elements
    return 10000 * y + 100 * x + sum;
}

static_assert(sizeof(coordinate) == 2 * sizeof(int));
static_assert(before() == 1020);
static_assert(after() == 201030);
```

The session's first candidate used `if constexpr` inside `get`. The array-form
reduction above was subsequently compiled successfully with the same warnings
at C++17, C++20, and C++23, O0 and O2, on local GCC 13.3.

### Evidence and standards audit

Local GCC 13.3 accepted the original candidate with `-std=c++23 -Wall -Wextra
-pedantic-errors -fsyntax-only` at both O0 and O2. It accepted the reduced source
above at C++17, C++20, and C++23, O0 and O2. An earlier prototype without the
explicit incomplete-specialization declaration also passed, but the retained
form declares the specialization before any use and gives the cleaner standards
argument.

The relevant current-draft rules were read directly:

- [Structured bindings](https://eel.is/c++draft/dcl.struct.bind) makes
  completeness plus `value` the tuple-protocol gate, then specifies ADL-only
  `get<I>` lookup and otherwise direct-member binding.
- [Explicit specializations](https://eel.is/c++draft/temp.expl.spec) says a
  declared but undefined class-template specialization can be used like another
  incomplete class and that no implicit instantiation is generated for it.
- [Namespace `std`](https://eel.is/c++draft/namespace.std) permits a program to
  specialize a standard-library class template when the declaration depends on
  a program-defined type and meets the template's requirements.

Targeted searches for incomplete `tuple_size` specializations completed between
structured bindings, or for a struct changing decomposition mid-file, found
general structured-binding customization material but no verified exact match.
That limited search does not establish historical novelty.

Cross-compiler verification was unfinished at the September 9 handoff. A request
to send the new source to public Compiler Explorer for GCC 16.2 and Clang 22.1 was
blocked because research authorization did not explicitly authorize disclosure
of unpublished source to that service. Tor was asked for explicit permission;
none was received during that initial session. The machine had only GCC 13.3
(`g++`, `g++-13`, and `c++`) locally. Clang acceptance was then unverified.

No trick folder, root README entry, or roughly-Turing claim was made at that
handoff. The proposed next steps were authorized cross-compiler verification,
controls, and an assessment of the artistic bar. Possible names considered
informally included **Phase shift**, **Shape-shifter**, and **Tuple metamorphosis**;
none was selected in the initial session.

Follow-up status: the exact published reduced source has now passed GCC 16.2
and Clang 22.1.0 at C++17/O2, and the spell is retained as Phase shift. See
[note 19](19-phase-shift.md) for the source hash, local controls, bootstrap
variant, and limits; this does not establish a new isolated machine.

## Other surveyed seams without retained probes

The search also considered the following mechanisms. These were brainstorming
branches, not compiler-tested failures:

- inferred exception specifications and implicitly deleted special members as
  Boolean circuits; earlier Astral-heap work had already found only conjunction-
  like propagation, not a closed machine;
- defaulted equality and spaceship operators as compiler-generated folds or
  first-mismatch searches; user-defined element comparisons still do the work;
- virtual-base coalescing as type-set union and constructor-count deduplication;
  this returns to inheritance/layout and ordinary runtime machinery;
- ADL associated-namespace traversal and member lookup as graph search;
  hidden-friend state would return to established stateful metaprogramming;
- `alignas`, enum underlying-type selection, bit-field packing, and tail-padding
  reuse as arithmetic or bin-packing oracles; these are layout-family ideas and
  did not produce a distinct capability;
- `delete[]` destructor counts as a hidden allocation-length oracle; this is an
  allocation-cookie version of the already explored one-pointer-container theme;
- constexpr leak and double-delete rejection as a linear-resource or pointer-
  inequality checker; this again consumes heap/lifetime evaluator metadata and
  is adjacent to Astral heap;
- virtual dispatch and dynamic type as metadata, source-location default
  arguments as hidden caller input, lambda/value-category metadata, implicit
  conversions, rewritten comparison candidates, coroutines, preprocessor
  recursion, linker identical-code folding, and string-literal identity;
  none yielded a small construction that was both new and stronger than the
  underlying well-known rule.

The central lesson from this session is stricter than “look for more compiler
metadata.” New invisible state is likely to collapse into an existing family.
The structured-binding candidate is interesting precisely because it instead
changes the language protocol selected for one unchanged type at different
program points.
