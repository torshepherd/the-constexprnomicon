// Runtime controls for Seance; C++20, x86-64 GCC/Clang.
// Exit zero means exactly the expected specializations initialized once.
// Deliberately assumes eager dynamic initialization, as the root spell does.

#include <cstdio>
#include <cstdlib>
#include <type_traits>

unsigned seen;
unsigned arrivals;

template<int Id>
inline int ghost = (++arrivals, seen |= (1u << Id), 0);

template<int Id>
auto summon() {
    std::abort(); // A real call would terminate; deduction never calls it.
    return ghost<Id>;
}

template<int Id>
int explicit_result() { return ghost<Id>; }

template<int Id>
auto local_static() {
    static int local = (++arrivals, seen |= (1u << Id), 0);
    return local;
}

template<class T, int Id>
concept haunted = requires {
    summon<Id>();
    typename T::missing;
};

template<class T, int Id>
concept warded = requires {
    typename T::missing;
    summon<Id>();
};

template<class T, int Id>
concept explicit_haunt = requires {
    explicit_result<Id>();
    typename T::missing;
};

template<class T>
requires haunted<T, 2>
int choose(T);
char choose(...); // Neither overload is defined or called.

// Core mechanism, duplicate queries, and lexical short-circuiting.
static_assert(!haunted<int, 0>);
static_assert(!haunted<double, 0>); // Different concept query, same ghost<0>.
static_assert(!warded<int, 1>);

// An actually rejected candidate still leaves evidence in ghost<2>.
static_assert(std::is_same_v<decltype(choose(42)), char>);

// An explicit return type avoids instantiating the function body.
static_assert(!explicit_haunt<int, 3>);

// Ordinary unevaluated contexts also require auto return-type deduction.
static_assert(sizeof(summon<4>()) == sizeof(int));
using inferred = decltype(summon<5>());
static_assert(std::is_same_v<inferred, int>);

// Merely mentioning the explicitly typed variable does not odr-use it.
static_assert(sizeof(ghost<6>) == sizeof(int));

// A function-local static waits for a real call; instantiation is insufficient.
static_assert(sizeof(local_static<7>()) == sizeof(int));

// Accepted queries work too: the concept has valid satisfying substitutions.
struct admitted { using missing = void; };
static_assert(haunted<admitted, 8>);

// Instantiating an ordinary false branch still instantiates its declarations.
template<class T>
void ordinary_if() {
    if (false) {
        static_assert(sizeof(summon<9 + (sizeof(T) == 0)>()) == sizeof(int));
    }
}

// A dependent discarded if-constexpr branch supplies the opposite control.
template<class T>
void discarded_if() {
    if constexpr (sizeof(T) == 0) {
        static_assert(sizeof(summon<10 + sizeof(T)>()) == sizeof(int));
    }
}

int main() {
    ordinary_if<int>();
    discarded_if<int>();
    constexpr unsigned expected = (1u << 0) | (1u << 2) | (1u << 4)
                                | (1u << 5) | (1u << 8) | (1u << 9);
    if (seen != expected || arrivals != 6) {
        std::printf("FAIL: seen=%u arrivals=%u\n", seen, arrivals);
        return 1;
    }
    std::puts("seance controls: six ghosts, zero calls");
}
