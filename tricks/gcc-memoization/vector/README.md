# Cache vector

[Source](vector.cpp)

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
[session notes](../../../.agents/notes/06-cache-memory.md#the-singleton-vector) for its
source hash and verification details.

## Try it

Recorded compiler evidence: **GCC 13.3 and 16.2, default constexpr depth/cache settings**, C++23.
Run from the repository root; executable names depend on your installation.

```sh
g++ -std=c++23 -O2 -c tricks/gcc-memoization/vector/vector.cpp -o /tmp/cache-vector.o
```

See [shared provenance](../../../docs/PROVENANCE.md) for known ingredients and related work.

## Typed cache experiments

The cache's 64-bit value restriction is a choice of encoding. Follow-up
[typed experiments](../../../.agents/notes/07-typed-cache.md) passed on GCC 16.2 with
double, enum, and 24-byte struct values, and with an owning `std::string`
serialized as length and characters. The small dictionary/vector examples still
use unsigned 64-bit values. The broader sketches require values that can be
encoded and reconstructed in constant evaluation; raw byte encoding does not
automatically handle padding, pointers, or arbitrary object identity.

For the complete codec, proxy, and iterator application, read the
[sortable cache vector](../sortable-vector/README.md).
