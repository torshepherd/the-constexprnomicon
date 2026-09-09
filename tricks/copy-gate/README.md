# Copy gate

[Source](copy-gate.cpp) · [Research handoff](../../.agents/notes/09-copy-gate.md)

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
matches. The [working notes](../../.agents/notes/09-copy-gate.md#equality-controls)
record the exact controls.

The associated [copy-driven factorial](../../.agents/notes/09-copy-gate.md#copy-driven-return-type-computation)
uses several techniques: `if constexpr` supplies decisions, ordinary constexpr
code reads and updates state, and return-type deduction drives recursive
instantiation. It does not establish a roughly-Turing machine using hidden
copying alone. Tor's [Pedantics](../../docs/Pedantics.md) requires each
internal operation to stay within an explicitly chosen set of techniques.
“Hidden copy plus overload resolution” is a possible set to investigate;
recursion within that set has not been demonstrated. The
[corrected assessment](../../.agents/notes/09-copy-gate.md#roughly-turing-assessment)
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

## Try it

Recorded compiler evidence: **GCC 13.3; changing-copy acceptance is compiler-dependent**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++23 -O2 -Wall -Wextra -pedantic-errors -c tricks/copy-gate/copy-gate.cpp -o /tmp/copy-gate.o
```
