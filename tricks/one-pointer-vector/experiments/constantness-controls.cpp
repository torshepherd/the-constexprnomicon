// PROBE=0 passes; PROBE=1..4 intentionally reject. See research note 22.
#include <type_traits>
#ifndef PROBE
#define PROBE 0
#endif
constexpr int a[]{10, 20, 0, 40};
constexpr int sum(const int* p) {
    int total=0;
#if PROBE == 0
    while (__builtin_constant_p(*p)) total += *p++;
#elif PROBE == 1
    while (std::is_constant_evaluated()) total += *p++;
#elif PROBE == 2
    if consteval { while (true) total += *p++; }
#elif PROBE == 3
    if constexpr (*p == 10) return 70;
#elif PROBE == 4
    while (requires { *p; }) total += *p++;
#endif
    return total;
}
static_assert(sum(a)==70);
