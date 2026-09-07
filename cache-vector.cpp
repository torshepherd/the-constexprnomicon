// GCC 13.3 / 16.2, C++23, default constexpr depth/cache settings.
// A const, empty global handle to a mutable vector in the compiler cache.
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

struct vector {
    struct reference {
        int index, line;
        consteval operator unsigned long long() const { return recall(index, line); }
        consteval auto operator=(unsigned long long value) const {
            return remember(index, value, -line);
        }
    };

    consteval auto size(int line = __builtin_LINE()) const { return recall(-1, line); }
    consteval void push_back(unsigned long long value, int line = __builtin_LINE()) const {
        int n = recall(-1, -line);
        remember(n, value, line);
        remember(-1, n + 1, line);
    }
    consteval void pop_back(int line = __builtin_LINE()) const {
        remember(-1, recall(-1, -line) - 1, line);
    }
    consteval void clear(int line = __builtin_LINE()) const { remember(-1, 0, line); }
    consteval reference operator[](int index, int line = __builtin_LINE()) const {
        return {index, line};
    }
};

inline constexpr vector v;

static_assert(__is_empty(vector) && sizeof(vector) == 1);
static_assert(v.size() == 0);
static_assert((v.push_back(42), v.size()) == 1);
static_assert(v[0] == 42);
static_assert((v.push_back(99), v.size()) == 2);
static_assert(v[0] == 42 && v[1] == 99);
static_assert((v[0] = 7) == 7);
static_assert(v[0] == 7);
static_assert((v.pop_back(), v.size()) == 1);
static_assert((v.push_back(17), v.size()) == 2);
static_assert(v[1] == 17);
static_assert((v.clear(), v.size()) == 0);
static_assert((v.push_back(0), v.size()) == 1);
static_assert((v.push_back(~0ull), v.size()) == 2);
static_assert(v[0] == 0 && v[1] == ~0ull);

// Another empty handle sees the same singleton.
inline constexpr vector alias;
static_assert(alias.size() == 2);
static_assert((alias[0] = 123) == 123);
static_assert(v[0] == 123);

// A saved proxy also saves its read identity; it can become stale.
constexpr auto old = v[0];
static_assert(old == 123);
static_assert((v[0] = 456) == 456);
static_assert(old == 123);
static_assert(v[0] == 456);
