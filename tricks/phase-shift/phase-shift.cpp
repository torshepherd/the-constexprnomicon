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
