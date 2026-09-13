#include <type_traits>
#include <memory>
consteval bool test() {
    union U { int a; int b; } u{.a=1};
    if (!std::is_within_lifetime(&u.a) || std::is_within_lifetime(&u.b)) return false;
    u.b=2;
    return !std::is_within_lifetime(&u.a) && std::is_within_lifetime(&u.b);
}
static_assert(test());
#ifdef ONE_PAST
static_assert([]() consteval {
    int a[2]{1,2};
    return !std::is_within_lifetime(a+2); // Intentionally ill-formed.
}());
#endif
