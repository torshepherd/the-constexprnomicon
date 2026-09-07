// GCC 13.3 / 16.2, C++23, default constexpr depth/cache settings.
// An append-only past gives the compiler a writable memory.
constexpr int ember(int key, int revision, int bit, int depth = 520) {
    return depth ? ember(key, revision, bit, depth - 1) : 0;
}

consteval auto remember(int key, unsigned long long value, int = __builtin_LINE()) {
    int revision = 0;
    while (__builtin_constant_p(ember(key, revision, 64))) ++revision;
    for (int i = 0; i < 64; ++i)
        if (value >> i & 1) ember(key, revision, i, 500);
    return ember(key, revision, 64, 500), value;
}

consteval auto recall(int key, int = __builtin_LINE()) {
    int revision = 0;
    while (__builtin_constant_p(ember(key, revision, 64))) ++revision;
    unsigned long long value = 0;
    for (int i = 0; i < 64; ++i)
        value |= (0ull + __builtin_constant_p(ember(key, revision - 1, i))) << i;
    return value;
}

static_assert(recall(7) == 0);
static_assert(remember(7, 42) == 42);
static_assert(recall(7) == 42);
static_assert(remember(7, 99) == 99);
static_assert(recall(7) == 99);

static_assert(remember(-3, 0xfedcba9876543210ull) == 0xfedcba9876543210ull);
static_assert(remember(0, 1ull << 63) == 1ull << 63);
static_assert(recall(-3) == 0xfedcba9876543210ull);
static_assert(recall(0) == 1ull << 63);
static_assert(recall(7) == 99);
static_assert(remember(7, 0) == 0);
static_assert(recall(7) == 0);
static_assert(remember(7, ~0ull) == ~0ull);
static_assert(recall(7) == ~0ull);
static_assert(recall(8) == 0);

// Even a missing result can become a stale cache entry.
static_assert(recall(9, 9000) == 0);
static_assert(remember(9, 17) == 17);
static_assert(recall(9, 9000) == 0);
static_assert(recall(9, 9001) == 17);

// Replaying an identical write need not perform its side effects again.
static_assert(remember(9, 23, 9002) == 23);
static_assert(remember(9, 31) == 31);
static_assert(remember(9, 23, 9002) == 23);
static_assert(recall(9) == 31);

// Separate evaluations leave different facts available to type formation.
static_assert(remember(10, 3) == 3);
using before = char[recall(10)];
static_assert(remember(10, 5) == 5);
using after = char[recall(10)];
static_assert(sizeof(before) == 3 && sizeof(after) == 5);
