// Seance: a false concept leaves a runtime ghost.
// C++20; observed on x86-64 GCC and Clang with eager dynamic initialization.
// Build and RUN this spell: its compile-time checks cause the executable to
// print "boo" once, despite the empty main. See seance.md for the mechanism.

#include <cstdio>

template<class T>
inline int ghost = std::puts("boo");

template<class T>
auto summon() { return ghost<T>; }

template<class T>
concept haunted = requires {
    summon<T>();
    typename T::missing;
};

template<class T>
concept warded = requires {
    typename T::missing;
    summon<T>();
};

static_assert(!haunted<int>);    // False, but instantiates the ghost for int.
static_assert(!haunted<int>);    // The same ghost does not initialize twice.
static_assert(!warded<double>); // False before summon is reached: no ghost.

int main() {}
