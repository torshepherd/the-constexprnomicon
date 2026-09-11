#include <tuple>
#include <cstddef>
struct coordinate { int x, y; };
namespace std { template<> struct tuple_size<coordinate>; }

template<class Dummy>
constexpr int frozen(coordinate p) {
    auto [x, y] = p;
    return x * 100 + y;
}

auto late = [](auto p) {
    auto [y, x, sum] = p;
    return y * 10000 + x * 100 + sum;
};

namespace std {
template<> struct tuple_size<coordinate> : integral_constant<size_t, 3> {};
template<size_t I> struct tuple_element<I, coordinate> { using type = int; };
}
template<std::size_t I> constexpr int get(coordinate p) {
    int a[]{p.y, p.x, p.x+p.y};
    return a[I];
}
static_assert(frozen<void>({10,20}) == 1020);
static_assert(late(coordinate{10,20}) == 201030);
