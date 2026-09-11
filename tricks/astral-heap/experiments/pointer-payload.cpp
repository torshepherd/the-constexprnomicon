// GCC, C++20 or later. A 65-bit payload in one eight-byte pointer.
// The eight shared arrays are an immutable codebook, not per-value storage.
constexpr auto extent = 1ull << 62;
struct realm { unsigned char bytes[extent]; };
struct word65 { bool high; unsigned long long low; };

constexpr unsigned char* encode(realm* const (&worlds)[8], word65 value) {
    auto bank = (unsigned(value.high) << 2) | (value.low >> 62);
    return worlds[bank]->bytes + (value.low & (extent - 1));
}

constexpr word65 decode(realm* const (&worlds)[8], const unsigned char* p) {
    for (unsigned bank = 0; bank < 8; ++bank)
        if (__builtin_constant_p(p - worlds[bank]->bytes)) {
            auto offset = static_cast<unsigned long long>(p - worlds[bank]->bytes);
            return {bool(bank >> 2), (static_cast<unsigned long long>(bank & 3) << 62) | offset};
        }
    throw "pointer is not in the codebook";
}

static_assert(sizeof(unsigned char*) == 8 && __CHAR_BIT__ == 8);
static_assert(sizeof(unsigned long long) == 8 && sizeof(realm) == extent);

consteval bool pointer_payload() {
    realm* worlds[8];
    for (auto& world : worlds) world = new realm{};

    bool okay = true;
    // All eight banks, boundaries, and every single offset bit.
    for (unsigned bank = 0; bank < 8; ++bank)
        for (unsigned k = 0; k < 64; ++k) {
            auto offset = k == 62 ? 0 : k == 63 ? extent - 1 : 1ull << k;
            word65 expected{bool(bank >> 2), (static_cast<unsigned long long>(bank & 3) << 62) | offset};
            auto p = encode(worlds, expected);
            auto copy = p;
            p = encode(worlds, {false, 0});
            auto actual = decode(worlds, copy);
            okay &= actual.high == expected.high && actual.low == expected.low;
            okay &= decode(worlds, p).low == 0 && !decode(worlds, p).high;
            okay &= *copy == 0; // No payload bytes were written.
        }

    auto a = encode(worlds, {false, 42});
    auto b = encode(worlds, {true, 42});
    okay &= a != b && !decode(worlds, a).high && decode(worlds, b).high;
    okay &= decode(worlds, a).low == 42 && decode(worlds, b).low == 42;
    auto swap = a;
    a = b;
    b = swap;
    okay &= decode(worlds, a).high && !decode(worlds, b).high;

    // The builtin safely declines the cross-allocation subtraction.
    okay &= !__builtin_constant_p(a - worlds[0]->bytes);
    okay &= __builtin_constant_p(a - worlds[4]->bytes);
#ifdef BAD_SUBTRACTION
    okay &= a - worlds[0]->bytes == 0; // Intentional rejection without the probe.
#endif
#ifdef BAD_PUN
    okay &= reinterpret_cast<unsigned long long>(a) != 0; // Still forbidden.
#endif
#ifdef UNKNOWN_POINTER
    auto outsider = new realm{};
    (void)decode(worlds, outsider->bytes); // Must reject rather than invent a bank.
    delete outsider;
#endif
    for (auto world : worlds) delete world;
    return okay;
}

static_assert(pointer_payload());
