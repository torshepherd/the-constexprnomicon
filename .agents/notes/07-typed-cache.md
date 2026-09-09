# Typed cache values — September 7, 2026

Tor asked whether the cache dictionary/vector were limited to integers or could
hold arbitrary `T`. The selected root spells still use unsigned 64-bit values;
this follow-up tested broader encodings without changing those selected sources.

## Finding

The mechanism is not limited to integers or 64 bits. A templated byte encoding
round-trips a double (including negative zero's sign bit), an enum, and a
24-byte three-double struct. A separate content encoding round-trips an owning
`std::string` across distinct constant evaluations: length 80, overwriting with
a different long string, clearing, and embedded nulls all pass.

There is no general arbitrary-object implementation here. The observable cache
primitive answers whether a known call is warm. It does not expose an API to
retrieve an unknown cached `T`. The working construction must encode enough
observable facts to reconstruct the value. Suitable constexpr serialization
and reconstruction can therefore extend the supported types. This is not a
proof that other approaches to retrieving cached objects are impossible.

Raw bytes do not automatically cover every trivially copyable type: padding can
be indeterminate, and pointer representations cannot be bit-cast in the tested
constant evaluations. Both negative controls below reject. Field-wise encoding
can avoid a struct's padding; the string demonstration encodes characters and
length instead of its representation or allocator state. Preserving arbitrary
pointer/reference identity, aliases, and ownership has not been achieved.

These are research sketches, not new generic container APIs. Earlier cache-depth,
recursion-depth, call-identity, stale-read, and replay limitations remain.

## Fixed-size typed memory

Compile this complete source with `g++ -std=c++23 -O0 -c`, or `-O2`. The negative
controls add `-DPOINTER` or `-DPADDING`. Every bit of `sizeof(T)` bytes gets a
cache position; position −1 marks a completed revision. There is no 64-bit limit.
The sketch assumes eight-bit bytes. Each `T` specialization has an independent
cache domain, so the checks intentionally reuse key zero.

An absent key returns `T{}` instead of interpreting all-zero bytes as a possibly
invalid value. This also requires a usable default constructor.

```cpp
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

template<class T> struct memory {
    static constexpr int ember(int key, int revision, int bit, int depth = 520) {
        return depth ? ember(key, revision, bit, depth - 1) : 0;
    }
    static consteval T put(int key, T value, int = __builtin_LINE()) {
        auto bytes = std::bit_cast<std::array<unsigned char, sizeof(T)>>(value);
        int revision = 0;
        while (__builtin_constant_p(ember(key, revision, -1))) ++revision;
        for (std::size_t i = 0; i < bytes.size() * 8; ++i)
            if (bytes[i / 8] >> (i % 8) & 1) ember(key, revision, i, 500);
        return ember(key, revision, -1, 500), value;
    }
    static consteval T get(int key, int = __builtin_LINE()) {
        int revision = 0;
        while (__builtin_constant_p(ember(key, revision, -1))) ++revision;
        if (!revision) return T{};
        std::array<unsigned char, sizeof(T)> bytes{};
        for (std::size_t i = 0; i < bytes.size() * 8; ++i)
            bytes[i / 8] |= __builtin_constant_p(ember(key, revision - 1, i)) << (i % 8);
        return std::bit_cast<T>(bytes);
    }
};

struct point { double x, y, z; constexpr bool operator==(const point&) const = default; };
enum class mood : unsigned { cursed = 0xfeed };
struct defaulted { int x = 42; };

static_assert(memory<double>::put(0, 3.25) == 3.25);
static_assert(memory<double>::get(0) == 3.25);
static_assert(memory<double>::put(0, -0.0) == 0.0);
static_assert(std::bit_cast<std::uint64_t>(memory<double>::get(0)) == 1ull << 63);
static_assert(memory<point>::put(0, {1.5, -7, 99.25}) == point{1.5, -7, 99.25});
static_assert(memory<point>::get(0) == point{1.5, -7, 99.25});
static_assert(memory<mood>::put(0, mood::cursed) == mood::cursed);
static_assert(memory<mood>::get(0) == mood::cursed);
static_assert(memory<defaulted>::get(0).x == 42);

#ifdef POINTER
inline constexpr int target = 42;
static_assert(memory<const int*>::put(7, &target) == &target);
static_assert(memory<const int*>::get(7) == &target);
#endif
#ifdef PADDING
struct padded { char c; int n; };
static_assert(memory<padded>::put(0, {'x', 42}).n == 42);
static_assert(memory<padded>::get(0).n == 42);
#endif
```

## Owning string as contents

For a separate translation unit, retain the includes and `memory<T>` definition
above, then replace all following checks and controls with this extension:

```cpp
#include <string>
#include <string_view>

consteval auto put_text(std::string text, int line = __builtin_LINE()) {
    for (int i = 0; i < int(text.size()); ++i) memory<char>::put(i, text[i], line);
    return memory<unsigned long long>::put(0, text.size(), line);
}
consteval std::string get_text(int line = __builtin_LINE()) {
    std::string text(memory<unsigned long long>::get(0, line), '\0');
    for (int i = 0; i < int(text.size()); ++i) text[i] = memory<char>::get(i, line);
    return text;
}

static_assert(put_text(std::string(80, 'x')) == 80);
static_assert([] consteval { return get_text() == std::string(80, 'x'); }());
static_assert(put_text("another string which is too long for SSO") == 40);
static_assert([] consteval { return get_text() == "another string which is too long for SSO"; }());
static_assert(put_text("") == 0);
static_assert([] consteval { return get_text().empty(); }());
static_assert(put_text(std::string("a\0b", 3)) == 3);
static_assert([] consteval { auto s = get_text(); return s.size()==3 && s[0]=='a' && s[1]==0 && s[2]=='b'; }());
```

The writer's temporary string is destroyed in its own constant evaluation.
The reader allocates a fresh result from the cached length and characters.
Nothing keeps the original allocation alive. Each consteval lambda compares
and destroys the reconstructed string inside the same outer evaluation, returning
only a bool. This does not export a dynamic string allocation into runtime storage.
The singleton string's character and length cache domains must not be reused for
unrelated purposes.

An initial scratch assertion incorrectly expected the second literal's length
to be 39. It was corrected to 40 before the successful recorded runs; that was
a test typo, not a cache or string restriction.

## Structured keys

This independent program shows that an ordinary by-value struct argument can
distinguish cached facts. Both its integer and floating-point fields affect
lookup without hashing them first. This establishes the tested key type, not
cache equivalence semantics for every class.

```cpp
struct key { int x; double y; };
constexpr int ember(key k, int depth = 520) {
    return depth ? ember(k, depth - 1) : 0;
}
static_assert(!__builtin_constant_p(ember({7, 3.5})));
static_assert(ember({7, 3.5}, 500) == 0);
static_assert(__builtin_constant_p(ember({7, 3.5})));
static_assert(!__builtin_constant_p(ember({8, 3.5})));
static_assert(!__builtin_constant_p(ember({7, 4.5})));
```

Recognizing that a candidate object is in the cache does not tell us what an
unknown cached object was. Richer keys alone do not solve arbitrary value retrieval.

## Compiler evidence

All configurations use C++23 and default constexpr depth/cache settings.
Local compiler: Ubuntu GCC 13.3.0 (`13.3.0-6ubuntu2~24.04`). CE compiler:
x86-64 GCC 16.2, `g162`, discovered earlier in this session.
No Clang runs were needed for this GCC-cache application.

| Probe | Local GCC 13.3.0 | CE GCC 16.2 |
| --- | --- | --- |
| Typed memory, positive checks | `-O0`: pass | `-O0`, `-O2`: pass |
| Same with `-DPOINTER` | `-O2`: rejects pointer bit-cast | `-O2`: rejects |
| Same with `-DPADDING` | `-O2`: rejects uninitialized byte read | `-O2`: rejects |
| Owning string encoding | `-O0`: pass | `-O0`, `-O2`: pass |
| Structured cache key | `-O0`: pass | `-O2`: pass |

Positive checks used `-fmax-errors=2`; negatives used `-fmax-errors=1`.
CE's negative diagnostics stop at a non-constant assertion; the local compiler
provides the more specific reasons in the table. Successful CE records have
code 0 and no diagnostics, and retain source text, flags, and SHA-256.

| Source snapshot | SHA-256 |
| --- | --- |
| Full typed-memory program above | `184f80b557d2ba5383777d2e1457f5cdd4dae9be4c1e9a2ed7ade1496389e87f` |
| Includes + memory definition + string extension | `49467f2d05a4a31c339bb5f0829a7167bb07ab54f9638c5d2821a46e1b4ff76a` |
| Structured-key program | `c9b73b7194557dc8152ba2e17f9022d1a815588bca2e19fa1a1a286d260511ef` |

The bit-cast restriction is consistent with the standard's
[bit-cast](https://eel.is/c++draft/bit.cast) and
[constant-evaluation](https://eel.is/c++draft/expr.const) rules. These links are
to the evolving draft; the experiments explicitly use C++23. No new pointer
serialization loophole was sought. The zero-storage owning vector remains tabled.

