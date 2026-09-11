// GCC, C++20. An 8-byte pointer carries 65 bits against a fixed tagged codebook.
// No compiler builtins, pointer punning, or writes to the codebook.
constexpr auto extent = 1ull << 62;
struct word65 { bool high; unsigned long long low; };
struct cell { const unsigned char bank; };

template<unsigned Bank>
struct realm {
    struct slot : cell { constexpr slot() : cell{Bank} {} };
    struct block { slot cells[extent]; };
    block* world = new block{};
    constexpr ~realm() { delete world; }
};

template<unsigned... Banks>
struct codebook : realm<Banks>... {
    constexpr cell const* encode(word65 value) const {
        auto bank = (unsigned(value.high) << 2) | (value.low >> 62);
        auto offset = value.low & (extent - 1);
        cell const* p = nullptr;
        ((bank == Banks ? p = this->realm<Banks>::world->cells + offset : p), ...);
        return p;
    }

    // Precondition: p was encoded by this live codebook.
    constexpr word65 decode(cell const* p) const {
        unsigned long long offset = 0;
        ((p->bank == Banks
            ? offset = static_cast<typename realm<Banks>::slot const*>(p)
                     - this->realm<Banks>::world->cells
            : offset), ...);
        return {bool(p->bank >> 2), (static_cast<unsigned long long>(p->bank & 3) << 62) | offset};
    }

    codebook() = default;
    codebook(codebook const&) = delete; // Copy encoded pointers, not the owner.
    codebook& operator=(codebook const&) = delete;
};

using atlas = codebook<0, 1, 2, 3, 4, 5, 6, 7>;
static_assert(sizeof(cell*) == 8 && sizeof(unsigned long long) == 8);
static_assert(sizeof(realm<7>::slot) == 1);
static_assert(sizeof(realm<7>::block) == extent);

consteval bool tagged_payload() {
    atlas worlds;
    bool okay = true;
    for (unsigned bank = 0; bank < 8; ++bank)
        for (unsigned k = 0; k < 64; ++k) {
            auto offset = k == 62 ? 0 : k == 63 ? extent - 1 : 1ull << k;
            word65 expected{bool(bank >> 2), (static_cast<unsigned long long>(bank & 3) << 62) | offset};
            auto p = worlds.encode(expected);
            auto copy = p;
            p = worlds.encode({false, 0});
            auto actual = worlds.decode(copy);
            okay &= actual.high == expected.high && actual.low == expected.low;
            okay &= copy->bank == bank;
            okay &= !worlds.decode(p).high && worlds.decode(p).low == 0;
        }

    auto a = worlds.encode({false, 42});
    auto b = worlds.encode({true, 42});
    okay &= a != b && !worlds.decode(a).high && worlds.decode(b).high;
    auto swap = a;
    a = b;
    b = swap;
    okay &= worlds.decode(a).high && !worlds.decode(b).high;
    okay &= worlds.decode(a).low == 42 && worlds.decode(b).low == 42;
    // Check untouched far tags in the same fixed codebook.
    okay &= worlds.realm<3>::world->cells[extent / 3].bank == 3;
    okay &= worlds.realm<7>::world->cells[extent / 3].bank == 7;
    return okay;
}
static_assert(tagged_payload());
