// A small observation workload for the patched GCC. Never run it at runtime.
constexpr auto extent = 1ull << 62;
struct realm { unsigned char bytes[extent]; };

consteval bool observe_astral_heap() {
    realm* worlds[8];
    for (auto& world : worlds) world = new realm{};
    for (unsigned i = 0; i < 8; ++i) {
        worlds[i]->bytes[0] = i + 1;
        worlds[i]->bytes[extent - 1] = i + 17;
    }
    auto p = worlds[6]->bytes + extent - 1;
    auto q = worlds[2]->bytes + extent - 1;
    bool distinct = p != q && *p == 23 && *q == 19;
    auto untouched = worlds[6]->bytes[extent / 2];
    for (auto world : worlds) delete world;
    return distinct && untouched == 0;
}
static_assert(sizeof(unsigned char*) == 8 && __CHAR_BIT__ == 8);
static_assert(observe_astral_heap());
