#include "tagged-pointer-payload.cpp"

// A normal decoded value may leave the evaluation; the allocated pointer may not.
consteval word65 result() {
    atlas worlds;
    auto p = worlds.encode({true, 0xfedcba9876543210ull});
#ifdef WRONG_BANK_CAST
    auto wrong = static_cast<realm<0>::slot const*>(p);
    return {false, wrong->bank}; // Intentional invalid downcast.
#elif defined(WRONG_CODEBOOK)
    atlas other;
    return other.decode(p); // Correct slot type, wrong allocation base.
#else
    return worlds.decode(p);
#endif
}
constexpr auto recovered = result();
static_assert(recovered.high && recovered.low == 0xfedcba9876543210ull);
