#include <tuple>
#include <cstddef>

struct coordinate { int x, y; };
namespace std { template<> struct tuple_size<coordinate>; }

#ifndef OMIT_EARLY
constexpr int before(const coordinate& p) {
    const auto& [x, y] = p;
    return x + y;
}
#endif

namespace std {
template<> struct tuple_size<coordinate> : integral_constant<size_t, 3> {};
template<size_t I> struct tuple_element<I, coordinate> { using type = int; };
}
template<std::size_t I> constexpr int get(const coordinate& p) {
    int a[]{p.y, p.x, p.x + p.y};
    return a[I];
}

constexpr bool after() {
    coordinate p{10, 20};
    auto& [a, b, c] = p;
    const auto& [x, y] = p;
    return a == 20 && b == 10 && c == 30 && x == 10 && y == 20;
}
static_assert(after());
