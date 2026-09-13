// Clang 22.1.0, -std=c++2c -stdlib=libc++. GCC trunk fails the value checks.
// The query is standard; empty-base replacement remains a compiler experiment.
// Do not copy: the state is in subobject lifetimes, not in object bytes.
#include <memory>
#include <type_traits>
#include <utility>

template<unsigned I> struct bit {};

template<unsigned... I> struct bits : bit<I>... {
    consteval bits(unsigned long long n = 0) { set(n); }
    // Revive the bases before implicit base destruction.
    constexpr ~bits() { (std::construct_at(static_cast<bit<I>*>(this)), ...); }
    consteval unsigned long long get() const {
        return ((1ull * std::is_within_lifetime(static_cast<const bit<I>*>(this)) << I) | ...);
    }
    consteval void set(unsigned long long n) {
        auto set_bit = [&]<unsigned J>() consteval {
            auto p = static_cast<bit<J>*>(this);
            if (std::is_within_lifetime(p)) std::destroy_at(p);
            if (n >> J & 1) std::construct_at(p);
        };
        (set_bit.template operator()<I>(), ...);
    }
};

template<unsigned... I> auto make_bits(std::integer_sequence<unsigned, I...>) -> bits<I...>;
using integer = decltype(make_bits(std::make_integer_sequence<unsigned, 64>{}));

static_assert(sizeof(integer) == 1);
static_assert(std::is_empty_v<integer>);

static_assert([]() consteval {
    integer n{0x123456789abcdef0ull};
    if (n.get() != 0x123456789abcdef0ull) return false;
    n.set(0xfedcba9876543210ull);
    return n.get() == 0xfedcba9876543210ull;
}());

static_assert([]() consteval {
    integer a{0}, b{~0ull};
    if (a.get() || b.get() != ~0ull) return false;
    unsigned long long n = 1;
    for (int i = 0; i < 16; ++i) {
        n = n * 6364136223846793005ull + 1;
        a.set(n);
        if (a.get() != n || b.get() != ~0ull) return false;
    }
    a.set(0); a.set(0);
    b.set(0); b.set(~0ull); b.set(~0ull);
    return !a.get() && b.get() == ~0ull;
}());
