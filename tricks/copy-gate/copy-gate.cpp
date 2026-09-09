// GCC 13.3.0, C++23. Copying sorts; deduction admits only fixed points.
// Deliberately compiler-dependent: see README.md's template-initialization caveat.
#include <algorithm>
#include <cstddef>
#include <type_traits>

template<std::size_t N> struct letters {
    char text[N];
    constexpr letters(const char (&s)[N]) { std::copy_n(s, N, text); }
    constexpr letters(const letters& s) : letters(s.text) {
        std::sort(text, text + N - 1);
    }
};

template<letters> struct word {};
template<letters S> std::true_type sorted(word<S>);
std::false_type sorted(...);

static_assert(decltype(sorted(word<"abc">{}))::value);
static_assert(!decltype(sorted(word<"cab">{}))::value);
static_assert(!decltype(sorted(word<"acb">{}))::value);
static_assert(!decltype(sorted(word<"bac">{}))::value);
static_assert(!decltype(sorted(word<"bca">{}))::value);
static_assert(!decltype(sorted(word<"cba">{}))::value);

static_assert(decltype(sorted(word<"">{}))::value);
static_assert(decltype(sorted(word<"x">{}))::value);
static_assert(decltype(sorted(word<"aabbc">{}))::value);
static_assert(!decltype(sorted(word<"aabac">{}))::value);
static_assert(decltype(sorted(word<"abcdefghijklmnopqrstuvwxyz">{}))::value);
static_assert(!decltype(sorted(word<"abcdefghijklmnopqrstuvwxzy">{}))::value);

// The final terminator is excluded; embedded nulls are ordinary characters.
static_assert(!decltype(sorted(word<"abcd\0">{}))::value);
static_assert(decltype(sorted(word<"\0abcd">{}))::value);

// Direct construction preserves order. Passing an lvalue copies and sorts it.
constexpr letters scrambled{"cab"};
static_assert(!std::is_same_v<word<"cab">, word<"abc">>);
static_assert(std::is_same_v<word<scrambled>, word<"abc">>);
