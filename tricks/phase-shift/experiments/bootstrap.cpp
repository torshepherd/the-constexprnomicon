#include <tuple>
#include <cstddef>

struct coordinate { int x, y; };

namespace std {
template<> struct tuple_size<coordinate>;
template<> struct tuple_size<coordinate>
    : integral_constant<size_t, [] {
          auto [x, y] = coordinate{1, 2};
          return x + y;
      }()> {};

template<size_t I> struct tuple_element<I, coordinate> { using type = int; };
}

template<std::size_t I>
constexpr int get(coordinate p) {
    int values[]{p.y, p.x, p.x + p.y};
    return values[I];
}

constexpr bool test() {
    auto [y, x, sum] = coordinate{10, 20};
    return y == 20 && x == 10 && sum == 30;
}
static_assert(std::tuple_size<coordinate>::value == 3);
static_assert(test());
