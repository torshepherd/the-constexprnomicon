// GCC 16.2, C++23, default constexpr depth/cache settings.
// No builtins; still a GCC memoization exploit. Unique must change per query.
#include <type_traits>
constexpr int slow(int key, int depth=520) {
    return depth ? slow(key,depth-1) : key;
}
template<int Unique, int N=0> consteval int next() {
    if constexpr (N==8) return -1;
    else if constexpr (requires { typename std::integral_constant<int,slow(N)>; })
        return next<Unique,N+1>();
    else return slow(N,500);
}
#if defined(NO_CACHE)
static_assert(next<0>() == 0);
static_assert(next<1>() == 0);
#elif defined(DEEP_LIMIT)
static_assert(next<0>() == -1); // The bounded scan finds every cold key evaluable.
#else
static_assert(next<0>() == 0);
static_assert(next<1>() == 1);
static_assert(next<2>() == 2);
static_assert(next<3>() == 3);
static_assert(next<3>() == 3); // Reusing a specialization does not advance.
static_assert(next<4>() == 4);
#endif
