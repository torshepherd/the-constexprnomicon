// GCC, C++20 or later. Eight 4-EiB objects; eight-byte pointers.
struct realm { unsigned char bytes[1ull << 62]; };

consteval bool astral_heap() {
    realm* worlds[8];
    for (auto& world : worlds) world = new realm{};

    for (unsigned i = 0; i < 8; ++i) {
        worlds[i]->bytes[0] = i + 1;
        worlds[i]->bytes[(1ull << 62) - 1] = i + 17;
    }

    bool intact = true;
    for (unsigned i = 0; i < 8; ++i) {
        intact &= worlds[i]->bytes[0] == i + 1;
        intact &= worlds[i]->bytes[(1ull << 62) - 1] == i + 17;
        intact &= worlds[i]->bytes[1ull << 61] == 0;
        for (unsigned j = 0; j < i; ++j)
            intact &= worlds[i] != worlds[j];
    }

    for (auto world : worlds) delete world;
    return intact;
}

static_assert(sizeof(realm*) == 8 && sizeof(unsigned char*) == 8 && __CHAR_BIT__ == 8);
static_assert(sizeof(realm) == (1ull << 62));
static_assert(astral_heap());
