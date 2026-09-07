// GCC 16.2, -std=c++23, default constexpr depth/cache settings.
// The compiler's memoization cache is the counter's storage.
constexpr int slow(int key, int depth = 520) {
    return depth ? slow(key, depth - 1) : key;
}

consteval int next(int = __builtin_LINE()) {
    int n = 0;
    while (__builtin_constant_p(slow(n))) ++n;
    return slow(n, 500);
}

static_assert(next() == 0);
static_assert(next() == 1);
static_assert(next() == 2);
static_assert(next() == 3);

static_assert(next(9000) == 4);
static_assert(next(9000) == 4); // The wrapper itself is memoized, too.
static_assert(next(9001) == 5);
