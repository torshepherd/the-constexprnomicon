// Chromatic aberration: empty-base optimization colors a graph.
// C++20, x86-64 GCC / Clang with Itanium-style object layout.
// Each edge is an empty type shared by its two endpoints.

#include <cstddef>
#include <type_traits>

struct ab {}; struct bc {}; struct cd {}; struct de {}; struct ea {};

struct A : ab, ea {};
struct B : ab, bc {};
struct C : bc, cd {};
struct D : cd, de {};
struct E : de, ea {};

struct pentagon : A, B, C, D, E {};

// The member spelling exposes the individual colors through offsetof.
struct palette {
    [[no_unique_address]] A a;
    [[no_unique_address]] B b;
    [[no_unique_address]] C c;
    [[no_unique_address]] D d;
    [[no_unique_address]] E e;
};

static_assert(std::is_empty_v<pentagon>);
static_assert(sizeof(pentagon) == 3);

static_assert(std::is_standard_layout_v<palette>);
static_assert(sizeof(palette) == 3);
static_assert(offsetof(palette, a) == 0);
static_assert(offsetof(palette, b) == 1);
static_assert(offsetof(palette, c) == 0);
static_assert(offsetof(palette, d) == 1);
static_assert(offsetof(palette, e) == 2);
