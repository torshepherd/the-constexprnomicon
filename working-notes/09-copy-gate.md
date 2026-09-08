# Copy gate — September 8, 2026

Tor initially offered several continuation directions, then explicitly asked for
a new thread and a new technique. We left the compiler-cache containers and
tabled pointer work alone. The selected [copy gate](../copy-gate.cpp) explores
class-valued template arguments with value-changing copy constructors.

## The selected construction

`letters<N>` stores a character array. Construction from a literal copies its
contents unchanged; copy construction sorts all but the last character, which
the sketch treats as the terminating null. `word<S>` is an empty token type.

```cpp
template<letters S> std::true_type sorted(word<S>);
std::false_type sorted(...);
```

The declarations have no constraints or bodies. On local GCC 13.3, deducing the
template argument from `word<"cab">` and rebuilding the function parameter type
produces `word<"abc">`. Those types cannot convert to one another, so the
ellipsis overload wins. `word<"abc">` stays unchanged and gets the typed
overload. This is overload viability testing a fixed point of normalization.
It does not search for a fixed point or normalize the original token in place.

The entire call can occur inside `decltype`. Template argument construction
executes the copy constructor and sorting while resolving an unevaluated call;
the `sorted` function itself is never executed or defined. A direct literal
retains its spelling, while forwarding a named `letters` lvalue copies and
normalizes it. The final two type-identity assertions distinguish those routes.

## Verification

Compiler: Ubuntu GCC 13.3.0 (`13.3.0-6ubuntu2~24.04`), local libstdc++.
Final source SHA-256:

```text
4593a265fee831443d31b5a0a852004dd258d081eb17f0751c8f13a8f03d3d3d
```

All three full-source commands passed with exit 0 and no diagnostics:

```sh
g++ -std=c++23 -O0 -Wall -Wextra -pedantic-errors -fmax-errors=2 -c copy-gate.cpp -o /tmp/copy-gate-O0.o
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -fmax-errors=2 -c copy-gate.cpp -o /tmp/copy-gate-O2.o
g++ -std=c++23 -O2 -fconstexpr-cache-depth=0 -Wall -Wextra -pedantic-errors -fmax-errors=2 -c copy-gate.cpp -o /tmp/copy-gate-no-cache.o
```

The source checks all six permutations of `abc`, empty and singleton strings,
duplicates, a 26-character sorted word and an adjacent inversion, embedded nulls,
and direct versus lvalue-forwarded template identity. There are no builtin
constant probes or compiler-state storage primitives in this spell. Disabling
constexpr caching confirms it does not depend on the earlier memoization trick.

Two isolated controls passed/failed as intended at C++23 `-O0`, with the same
warning and pedantic flags:

- Remove only `std::sort` from the copy constructor: both `abc` and `cab` select
  the typed overload.
- Keep sorting and remove the fallback: `cab` fails with a diagnostic saying it
  cannot convert `word<letters<4>{"cab"}>` to `word<letters<4>{"abc"}>`.

The controls preceded a cosmetic reduction of the selected copy constructor
to delegate to the literal constructor. Both versions copy the characters and
then sort; the full checks above bind the final reduced source.

No Clang, newer-GCC, or Godbolt run was made. Do not generalize local acceptance.
There is no pending compiler request or bug report.

## Prior art and standard boundary

The initial independent probes used an integer copy counter. A targeted search
then found Jelle Hellings's December 26, 2022 article,
[A buggy adventure: Non-type template parameters](https://jhellings.nl/article?articleid=1),
which already studies changing copy constructors and differences among GCC,
Clang, and MSVC forwarding behavior. Credit it for the underlying ingredient;
the sorting gate and controls were assembled in this session. No claim of
historical novelty is established by the limited search.

Do not carry the article's interpretation of the older wording forward as a
current portability conclusion. [N4950's C++23 wording](https://timsong-cpp.github.io/cppwp/n4950/temp.arg.nontype)
describes a converted constant expression.
[CWG2459](https://cplusplus.github.io/CWG/issues/2459.html) records that template
parameter initialization was underspecified and was resolved by
[P2308R1](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2308r1.html),
adopted in November 2023. The paper introduces a temporary followed by
copy-initialization of a template parameter object, requiring that second copy
to preserve template-argument equivalence. It explicitly connects this
requirement to deduction, specialization, and the current instantiation.

Under that wording, constructing the parameter object from the unsorted
temporary in `word<"cab">` is already ill-formed: copying changes its contents.
The fallback is not a portable escape from that rule. GCC 13.3 accepts this
program even with `-pedantic-errors`; that is compiler evidence, not a standard
guarantee. The [current draft](https://eel.is/c++draft/temp.arg.nontype) contains
the equivalence-preserving initialization requirement.

## Earlier working probes

### A scalar gate

Before sorting strings, the following complete probe passed C++23 `-O0`:

```cpp
#include <type_traits>
struct odd {
    int n;
    constexpr odd(int n) : n(n) {}
    constexpr odd(const odd& x) : n(x.n | 1) {}
};
template<odd N> struct box {};
template<odd N> std::true_type accepts(box<N>);
std::false_type accepts(...);
static_assert(!decltype(accepts(box<4>{}))::value);
static_assert(decltype(accepts(box<5>{}))::value);
```

The copy maps an even value to the following odd value. Only values unchanged
by that mapping retain a matching function parameter type. With a counter that
always increments and no fallback, deduction from `box<3>` gave a failed
conversion from `box<tick{3}>` to `box<tick{4}>`.

### Copy-driven return-type computation

This separate source passed C++23 `-O0 -Wall -Wextra -pedantic-errors`:

```cpp
#include <type_traits>

struct product {
    int n, value = 1;
    constexpr product(int n) : n(n) {}
    constexpr product(const product& p) : n(p.n - 1), value(p.value * p.n) {}
};

template<product P> auto factorial() {
    if constexpr (P.n) return factorial<P>();
    else return std::integral_constant<int, P.value>{};
}

static_assert(decltype(factorial<0>())::value == 1);
static_assert(decltype(factorial<1>())::value == 1);
static_assert(decltype(factorial<5>())::value == 120);
static_assert(decltype(factorial<6>())::value == 720);
static_assert(decltype(factorial<10>())::value == 3628800);
```

The function is not `constexpr` and is never called. Instantiating its deduced
return type repeatedly forwards `P`; the copy constructor decreases the
remaining count and accumulates the product. The apparent self-call denotes a
different specialization on this GCC. Template instantiation drives the loop;
constant evaluation still executes the constructors and branch conditions.
The tested input range avoids negative counts and integer overflow. Do not
confuse this with an arbitrary runtime function becoming constant-evaluable.

Scratch source hash (including its whitespace):
`f0e7a9eab85700dfeb9205ab3210546d8bc3f86ba5e2b2012dd66cdf8d68238c`.
The source above preserves the complete tested program. No O2 claim was made
for this separate probe; the selected sorting gate received the full checks.

### Current-instantiation and specialization traps

With a copy-decrementing `tick`, these two inheritance attempts were rejected
immediately as recursive types:

```cpp
template<tick T> struct chain : chain<T> {};
template<tick T> struct chain : chain<(T)> {};
```

Replacing the base argument with `(T, T)` made copying happen. With an explicit
`template<> struct chain<0> {};`, a check that `chain<0>` is a base of `chain<4>`
passed. A constrained partial specialization using `requires (T.n == 0)` did
not stop the chain in the tested form: compilation hit the deliberately low
template-depth limit of 30 and displayed negative tick values. Substitution and
specialization are additional copying sites; do not assume only the recursive
step invokes the constructor.

A separate copy-incrementing class probe passed with `box<0>::self::value == 0`
for `using self = box<N>;`, but `box<0>::next::value == 1` and
`box<0>::next::next::value == 2` for `using next = box<(N, N)>;`.
GCC's treatment is consistent with a distinction involving the current
instantiation, but no compiler-source diagnosis was completed. Parentheses
alone did not change its observed treatment, despite the current draft's
[current-instantiation discussion](https://eel.is/c++draft/temp.dep.type).

## Other bounded probes from the initial sweep

- A self-pointing template argument whose copy resets its pointer to `this`
  was rejected as non-constant/incompletely initialized.
- A copy constructor that attempted to modify a namespace-scope constexpr
  source through `const_cast` was rejected.
- Direct escape of a consteval function pointer was rejected. Returning a
  union after replacing that pointer with an active integer member compiled,
  but that does not establish a callable escaped pointer; the inactive-member
  runtime read was not run or promoted as a working technique.
- GCC accepted local variable-length arrays inside a consteval function,
  including element writes and an element type with a constexpr destructor.
  A seven-element sum produced 21. Using their size as an NTTP inside the
  function still failed because the function argument was not a constant
  expression there. This is GCC's documented VLA extension, not a route from
  ordinary function parameters to arbitrary template arguments.
- `__builtin_dynamic_object_size` on a constexpr allocation was rejected as
  non-constant in the tested shape.

These were small local GCC 13.3 C++23 `-O0` probes. Only the selected copy gate
is added to the root spell collection; the other results are handoff material.
