# Sortable cache vector

[Advanced source](sortable-vector.cpp) ·
[Research handoff](../../../.agents/notes/08-sortable-cache-vector.md)

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

## Fresh calls inside algorithms

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

## Sorting and swapping

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

## Limits and reproduction

The advanced source passed locally on Ubuntu GCC 13.3.0 with:

```sh
g++ -std=c++23 -O2 -fconstexpr-cache-depth=64 -c tricks/gcc-memoization/sortable-vector/sortable-vector.cpp -o /tmp/sortable-cache.o
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

Start with the [small integer vector](../vector/README.md) for the underlying trick.
