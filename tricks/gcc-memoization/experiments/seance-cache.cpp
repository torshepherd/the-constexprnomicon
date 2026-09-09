// Seance x GCC cache memory: bounded controls, not a new root spell.
// GCC 13.3 / 16.2, C++23 -O2, default constexpr limits.
// GCC 13.3 -O0 needs -DEXPECT_FOLDED=0; GCC 16.2 -O0 uses the defaults.
// See 15-seance.md for the folding difference and other controls.

#ifndef EXPECT_COLD
#define EXPECT_COLD 0
#endif
#ifndef EXPECT_WARM
#define EXPECT_WARM 1
#endif
#ifndef EXPECT_FOLDED
#define EXPECT_FOLDED 1
#endif

constexpr int slow(int key, int depth = 520) {
    return depth ? slow(key, depth - 1) : key;
}

template<int Key>
inline constexpr int ghost = slow(Key, 500);

template<int Key>
auto summon() { return ghost<Key>; }

template<int Key>
int explicit_result() { return ghost<Key>; }

template<class T, int Key>
concept haunted = requires {
    summon<Key>();
    typename T::missing;
};

template<class T, int Key>
concept warded = requires {
    typename T::missing;
    summon<Key>();
};

template<class T, int Key>
concept explicit_query = requires {
    explicit_result<Key>();
    typename T::missing;
};

// A nested requirement can force the same evaluation without Seance's helper.
template<class T, int Key>
concept direct = requires {
    requires (slow(Key, 500) == Key);
    typename T::missing;
};

template<class T, int Key>
concept merely_named = requires {
    slow(Key, 500);
    typename T::missing;
};

template<int Key>
consteval int immediate() { return slow(Key, 500); }

template<class T, int Key>
concept named_immediate = requires {
    immediate<Key>();
    typename T::missing;
};

// No required constant evaluation here. GCC's optional folding differs by -O.
template<int Key>
auto body_only() { return slow(Key, 500); }

template<class T, int Key>
concept instantiated_only = requires {
    body_only<Key>();
    typename T::missing;
};

static_assert(__builtin_constant_p(slow(7)) == EXPECT_COLD);
static_assert(!haunted<int, 7>);
static_assert(__builtin_constant_p(slow(7)) == EXPECT_WARM);

static_assert(!warded<int, 8>);
static_assert(__builtin_constant_p(slow(8)) == EXPECT_COLD);

static_assert(!explicit_query<int, 9>);
static_assert(__builtin_constant_p(slow(9)) == EXPECT_COLD);

static_assert(__builtin_constant_p(slow(10)) == EXPECT_COLD);
static_assert(!direct<int, 10>);
static_assert(__builtin_constant_p(slow(10)) == EXPECT_WARM);

static_assert(!merely_named<int, 11>);
static_assert(__builtin_constant_p(slow(11)) == EXPECT_COLD);

static_assert(!named_immediate<int, 12>);
static_assert(__builtin_constant_p(slow(12)) == EXPECT_COLD);

static_assert(!instantiated_only<int, 13>);
static_assert(__builtin_constant_p(slow(13)) == EXPECT_FOLDED);
