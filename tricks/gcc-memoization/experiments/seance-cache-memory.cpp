// Seance x cache-memory.cpp: a failed concept writes compiler-resident memory.
// The three memory primitives below are copied unchanged from cache-memory.cpp.
// GCC 13.3 / 16.2, C++23 O0/O2, default constexpr depth/cache settings.
// Do not run the unbounded dictionary with an increased recursion limit.
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


template<class T, int Event>
inline constexpr auto ghost = remember(77, 42, Event);

template<class T, int Event>
auto summon() { return ghost<T, Event>; }

template<class T, int Event>
concept rejected = requires {
    summon<T, Event>();
    typename T::missing;
};

template<class T, int Event>
concept direct = requires {
    requires (remember(77, 99, Event) == 99);
    typename T::missing;
};

static_assert(recall(77) == 0);
static_assert(!rejected<int, 1000>);
static_assert(recall(77) == 42);
static_assert(!direct<int, 1001>);
static_assert(recall(77) == 99);
// Same query is not a repeated write.
static_assert(!rejected<int, 1000>);
static_assert(recall(77) == 99);
// Even a fresh type can replay the old memoized remember call.
static_assert(!rejected<double, 1000>);
static_assert(recall(77) == 99);
// Fresh query, fresh writer identity.
static_assert(!rejected<int, 1002>);
static_assert(recall(77) == 42);
