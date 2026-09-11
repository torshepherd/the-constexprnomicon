#include <cstddef>
#include <tuple>

struct coordinate { int x; int y; };
namespace std { template<> struct tuple_size<coordinate>; }

// Its argument type is dependent: the body is instantiated only at the call.
constexpr auto late = [](auto point) {
    auto [y, x, sum] = point;
    return 10000 * y + 100 * x + sum;
};

namespace std {
template<> struct tuple_size<coordinate> : integral_constant<size_t, 3> {};
template<size_t I> struct tuple_element<I, coordinate> { using type = int; };
}

template<std::size_t I>
constexpr int get(coordinate point) {
    int values[]{point.y, point.x, point.x + point.y};
    return values[I];
}

static_assert(late(coordinate{10, 20}) == 201030);
