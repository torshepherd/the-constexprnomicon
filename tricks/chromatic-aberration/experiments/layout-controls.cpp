// Controls for chromatic-aberration.cpp. Same target/layout assumptions.
#include <cstddef>
#include <type_traits>

namespace order_matters {
struct ab {}; struct bc {}; struct cd {};
struct A : ab {};
struct B : ab, bc {};
struct C : bc, cd {};
struct D : cd {};

// The same four-vertex path, in two different greedy traversal orders.
struct good : A, B, C, D {};
struct bad  : A, D, B, C {};
struct colors {
    [[no_unique_address]] A a;
    [[no_unique_address]] D d;
    [[no_unique_address]] B b;
    [[no_unique_address]] C c;
};

static_assert(sizeof(good) == 2);
static_assert(sizeof(bad) == 3); // Not the graph's chromatic number!
static_assert(std::is_standard_layout_v<colors>);
static_assert(offsetof(colors, a) == 0);
static_assert(offsetof(colors, d) == 0);
static_assert(offsetof(colors, b) == 1);
static_assert(offsetof(colors, c) == 2);
}

namespace overlap_matters {
struct ab {}; struct bc {}; struct cd {}; struct de {}; struct ea {};
struct A : ab, ea {};
struct B : ab, bc {};
struct C : bc, cd {};
struct D : cd, de {};
struct E : de, ea {};

// Ordinary members consume space instead of reusing offsets as colors.
struct solid { A a; B b; C c; D d; E e; };

static_assert(std::is_standard_layout_v<solid>);
static_assert(sizeof(solid) == 5);
static_assert(offsetof(solid, a) == 0);
static_assert(offsetof(solid, b) == 1);
static_assert(offsetof(solid, c) == 2);
static_assert(offsetof(solid, d) == 3);
static_assert(offsetof(solid, e) == 4);
}
