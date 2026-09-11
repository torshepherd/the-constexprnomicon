#include <cstddef>
#include <tuple>

struct coordinate { int x; int y; };
namespace std { template<> struct tuple_size<coordinate>; }

constexpr bool before(coordinate& point) {
#ifdef EARLY_THREE
    auto& [x, y, sum] = point; // must fail: still two real members
    (void)sum;
#else
    auto& [x, y] = point;
#endif
    x += 1;
    return &x == &point.x && &y == &point.y;
}

namespace std {
template<> struct tuple_size<coordinate> : integral_constant<size_t, 3> {};
template<size_t I> struct tuple_element<I, coordinate> { using type = int; };
}

#ifndef OMIT_GET
template<std::size_t I>
constexpr decltype(auto) get(coordinate& point) {
    if constexpr (I == 0) return (point.y);
    else if constexpr (I == 1) return (point.x);
    else return point.x + point.y;
}
#endif

constexpr bool after(coordinate& point) {
#ifdef LATE_TWO
    auto& [y, x] = point; // must fail: now three tuple elements
    return &x == &point.x && &y == &point.y;
#else
    auto& [y, x, sum] = point;
    const int snapshot = point.x + point.y;
    x += 2;
    y += 3;
    return &x == &point.x && &y == &point.y && sum == snapshot;
#endif
}

constexpr bool test() {
    coordinate point{10, 20};
    return before(point) && after(point) && point.x == 13 && point.y == 23;
}

static_assert(sizeof(coordinate) == 2 * sizeof(int));
static_assert(test());
