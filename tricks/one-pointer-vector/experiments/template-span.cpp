// GCC 16.2 / Clang 22.1.0, C++23. The pointer is a template argument.
#include <type_traits>
constexpr int a[]{10,20,0,40};
template<const int* P, unsigned I=0> consteval unsigned count() {
    if constexpr (requires { typename std::integral_constant<int, P[I]>; })
        return 1 + count<P,I+1>();
    else return 0;
}
static_assert(count<a>()==4);
