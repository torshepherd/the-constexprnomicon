// Four XOR truth-table rows, evaluated by object layout alone.
// Each namespace encodes zero, x, y, NAND(x,y), NAND(x,xy),
// NAND(y,xy), and NAND(left,right), in that order.
// Inputs become true by sharing a marker with the zero seed.
// Zero offsets mean false; any nonzero offset means true.
#include <cstddef>
#include <type_traits>
namespace xy00 {
struct e13 {};
struct e23 {};
struct e14 {};
struct e34 {};
struct e25 {};
struct e35 {};
struct e46 {};
struct e56 {};
struct v0 {};
struct v1 : e13, e14 {};
struct v2 : e23, e25 {};
struct v3 : e13, e23, e34, e35 {};
struct v4 : e14, e34, e46 {};
struct v5 : e25, e35, e56 {};
struct v6 : e46, e56 {};
struct circuit {
[[no_unique_address]] v0 zero;
[[no_unique_address]] v1 x;
[[no_unique_address]] v2 y;
[[no_unique_address]] v3 xy;
[[no_unique_address]] v4 left;
[[no_unique_address]] v5 right;
[[no_unique_address]] v6 out;
};
static_assert((offsetof(circuit,x) != 0) == false);
static_assert((offsetof(circuit,y) != 0) == false);
static_assert((offsetof(circuit,xy) != 0) == true);
static_assert((offsetof(circuit,out) != 0) == false);
static_assert(std::is_standard_layout_v<circuit>);
}
namespace xy01 {
struct e13 {};
struct e23 {};
struct e14 {};
struct e34 {};
struct e25 {};
struct e35 {};
struct e46 {};
struct e56 {};
struct e02 {};
struct v0 : e02 {};
struct v1 : e13, e14 {};
struct v2 : e23, e25, e02 {};
struct v3 : e13, e23, e34, e35 {};
struct v4 : e14, e34, e46 {};
struct v5 : e25, e35, e56 {};
struct v6 : e46, e56 {};
struct circuit {
[[no_unique_address]] v0 zero;
[[no_unique_address]] v1 x;
[[no_unique_address]] v2 y;
[[no_unique_address]] v3 xy;
[[no_unique_address]] v4 left;
[[no_unique_address]] v5 right;
[[no_unique_address]] v6 out;
};
static_assert((offsetof(circuit,x) != 0) == false);
static_assert((offsetof(circuit,y) != 0) == true);
static_assert((offsetof(circuit,xy) != 0) == true);
static_assert((offsetof(circuit,out) != 0) == true);
static_assert(std::is_standard_layout_v<circuit>);
}
namespace xy10 {
struct e13 {};
struct e23 {};
struct e14 {};
struct e34 {};
struct e25 {};
struct e35 {};
struct e46 {};
struct e56 {};
struct e01 {};
struct v0 : e01 {};
struct v1 : e13, e14, e01 {};
struct v2 : e23, e25 {};
struct v3 : e13, e23, e34, e35 {};
struct v4 : e14, e34, e46 {};
struct v5 : e25, e35, e56 {};
struct v6 : e46, e56 {};
struct circuit {
[[no_unique_address]] v0 zero;
[[no_unique_address]] v1 x;
[[no_unique_address]] v2 y;
[[no_unique_address]] v3 xy;
[[no_unique_address]] v4 left;
[[no_unique_address]] v5 right;
[[no_unique_address]] v6 out;
};
static_assert((offsetof(circuit,x) != 0) == true);
static_assert((offsetof(circuit,y) != 0) == false);
static_assert((offsetof(circuit,xy) != 0) == true);
static_assert((offsetof(circuit,out) != 0) == true);
static_assert(std::is_standard_layout_v<circuit>);
}
namespace xy11 {
struct e13 {};
struct e23 {};
struct e14 {};
struct e34 {};
struct e25 {};
struct e35 {};
struct e46 {};
struct e56 {};
struct e01 {};
struct e02 {};
struct v0 : e01, e02 {};
struct v1 : e13, e14, e01 {};
struct v2 : e23, e25, e02 {};
struct v3 : e13, e23, e34, e35 {};
struct v4 : e14, e34, e46 {};
struct v5 : e25, e35, e56 {};
struct v6 : e46, e56 {};
struct circuit {
[[no_unique_address]] v0 zero;
[[no_unique_address]] v1 x;
[[no_unique_address]] v2 y;
[[no_unique_address]] v3 xy;
[[no_unique_address]] v4 left;
[[no_unique_address]] v5 right;
[[no_unique_address]] v6 out;
};
static_assert((offsetof(circuit,x) != 0) == true);
static_assert((offsetof(circuit,y) != 0) == true);
static_assert((offsetof(circuit,xy) != 0) == false);
static_assert((offsetof(circuit,out) != 0) == false);
static_assert(std::is_standard_layout_v<circuit>);
}
