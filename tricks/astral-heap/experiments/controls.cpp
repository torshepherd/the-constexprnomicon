// PROBE=0: positive controls. PROBE=1..5: intentional rejection controls.
#ifndef PROBE
#define PROBE 0
#endif
#ifndef ADDRESS_BITS
#define ADDRESS_BITS 64
#endif

constexpr auto extent = 1ull << (ADDRESS_BITS - 2);
struct realm { unsigned char bytes[extent]; };
static_assert(sizeof(unsigned char*) * __CHAR_BIT__ == ADDRESS_BITS);
static_assert(sizeof(realm) == extent);

consteval bool controls() {
    realm* worlds[8];
    for (auto& world : worlds)
#if PROBE == 1
        world = new realm;  // No zero initialization.
#else
        world = new realm{};
#endif

    bool okay = true;
    for (unsigned i = 0; i < 8; ++i) {
        auto p = worlds[i]->bytes;
        p[0] = i + 1;
        p[extent / 2 + 123] = i + 41;
        p[extent - 1] = i + 81;
    }
    for (unsigned i = 0; i < 8; ++i) {
        auto p = worlds[i]->bytes;
        okay &= p[0] == i + 1 && p[extent - 1] == i + 81;
        okay &= p[extent / 2 + 123] == i + 41;
        okay &= p[1] == 0 && p[extent / 2] == 0 && p[extent - 2] == 0;
        okay &= (p + extent - 1) - p == extent - 1;
        p[extent / 2 + 123] = 200 - i;
        okay &= p[extent / 2 + 123] == 200 - i;
        p[extent / 2 + 123] = 0;
        okay &= p[extent / 2 + 123] == 0;
        for (unsigned j = 0; j < i; ++j) {
            okay &= worlds[i] != worlds[j];
            okay &= p != worlds[j]->bytes;
            okay &= p + extent - 1 != worlds[j]->bytes + extent - 1;
            okay &= p != worlds[j]->bytes + extent / 2;
        }
    }

    *worlds[7] = *worlds[0];  // A sparse 4-EiB copy on the 64-bit target.
    worlds[0]->bytes[0] = 99;
    okay &= worlds[7]->bytes[0] == 1 && worlds[0]->bytes[0] == 99;
    okay &= worlds[7]->bytes[extent - 1] == 81;
    okay &= worlds[7]->bytes[extent / 2] == 0;

#if PROBE == 2
    okay &= worlds[0]->bytes[extent] == 0;  // Out of bounds.
#elif PROBE == 4
    okay &= reinterpret_cast<unsigned long long>(worlds[0]) != 0;
#endif
#if PROBE != 3
    for (auto world : worlds) delete world;
#endif
    return okay;
}

#if PROBE == 5
consteval realm* escape() { return new realm{}; }
constexpr auto escaped = escape();
#else
static_assert(controls());
#endif
