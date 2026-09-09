// GCC 13.3.0: -std=c++23 -fconstexpr-cache-depth=64, default depth limit.
// Local empty handles keep algorithm reads fresh; the cache owns the contents.
#include <algorithm>
#include <array>
#include <bit>
#include <compare>
#include <iterator>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

using bytes = std::vector<unsigned char>;

template<class T> struct binary {
    static constexpr bytes encode(const T& value) {
        auto b = std::bit_cast<std::array<unsigned char, sizeof(T)>>(value);
        return {b.begin(), b.end()};
    }
    static constexpr T decode(const bytes& b) {
        if (b.size() != sizeof(T)) throw "wrong encoded size";
        std::array<unsigned char, sizeof(T)> a{};
        std::copy(b.begin(), b.end(), a.begin());
        return std::bit_cast<T>(a);
    }
};

struct text {
    static constexpr bytes encode(const std::string& value) {
        return {value.begin(), value.end()};
    }
    static constexpr std::string decode(const bytes& b) {
        return {b.begin(), b.end()};
    }
};

template<class T, class Codec = binary<T>, class Tag = void> struct cache_vector {
    static constexpr int ember(int key, int revision, int bit, int depth = 640) {
        return depth ? ember(key, revision, bit, depth - 1) : 0;
    }
    template<class U, class Encoding> static constexpr void put(
        int key, const U& value, const void* anchor) {
        (void)anchor;
        int revision = 0;
        while (__builtin_constant_p(ember(key, revision, -1))) ++revision;
        auto b = Encoding::encode(value);
        for (int i = 0; i < int(b.size()); ++i) {
            for (int bit = 0; bit < 8; ++bit)
                if (b[i] >> bit & 1) ember(key, revision, 9 * i + bit, 384);
            ember(key, revision, 9 * i + 8, 384);
        }
        ember(key, revision, -1, 384);
    }
    template<class U, class Encoding> static constexpr U get(int key, const void* anchor) {
        (void)anchor;
        int revision = 0;
        while (__builtin_constant_p(ember(key, revision, -1))) ++revision;
        if (!revision) {
            if constexpr (std::is_same_v<U, int>) if (key == -1) return 0;
            throw "unwritten element";
        }
        bytes b;
        for (int i = 0; __builtin_constant_p(ember(key, revision - 1, 9 * i + 8)); ++i) {
            unsigned char value = 0;
            for (int bit = 0; bit < 8; ++bit)
                value |= __builtin_constant_p(ember(key, revision - 1, 9 * i + bit)) << bit;
            b.push_back(value);
        }
        return Encoding::decode(b);
    }

    struct reference {
        const cache_vector* owner;
        int index;
        constexpr operator T() const { return get<T, Codec>(index, owner); }
        constexpr const reference& operator=(const T& value) const {
            put<T, Codec>(index, value, owner);
            return *this;
        }
        constexpr const reference& operator=(const reference& other) const {
            return *this = T(other);
        }
        friend constexpr void swap(reference a, reference b) {
            T old = a;
            a = T(b);
            b = old;
        }
        friend constexpr bool operator==(const reference& a, const reference& b) {
            return T(a) == T(b);
        }
        friend constexpr bool operator==(const reference& a, const T& b) { return T(a) == b; }
        friend constexpr auto operator<=>(const reference& a, const reference& b)
            requires std::three_way_comparable<T> { return T(a) <=> T(b); }
        friend constexpr auto operator<=>(const reference& a, const T& b)
            requires std::three_way_comparable<T> { return T(a) <=> b; }
    };

    struct iterator {
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using iterator_concept = std::random_access_iterator_tag;
        using iterator_category = std::random_access_iterator_tag;
        const cache_vector* owner = nullptr;
        int index = 0;
        constexpr reference operator*() const { return {owner, index}; }
        constexpr reference operator[](difference_type n) const { return *(*this + n); }
        constexpr iterator& operator++() { ++index; return *this; }
        constexpr iterator& operator--() { --index; return *this; }
        constexpr iterator operator++(int) { auto old = *this; ++*this; return old; }
        constexpr iterator operator--(int) { auto old = *this; --*this; return old; }
        constexpr iterator& operator+=(difference_type n) { index += n; return *this; }
        constexpr iterator& operator-=(difference_type n) { index -= n; return *this; }
        friend constexpr iterator operator+(iterator i, difference_type n) { return i += n; }
        friend constexpr iterator operator+(difference_type n, iterator i) { return i += n; }
        friend constexpr iterator operator-(iterator i, difference_type n) { return i -= n; }
        friend constexpr difference_type operator-(iterator a, iterator b) { return a.index - b.index; }
        constexpr auto operator<=>(const iterator& other) const { return index <=> other.index; }
        constexpr bool operator==(const iterator& other) const { return index == other.index; }
        friend constexpr T iter_move(iterator i) { return T(*i); }
        friend constexpr void iter_swap(iterator a, iterator b) { swap(*a, *b); }
    };

    constexpr int size() const { return get<int, binary<int>>(-1, this); }
    constexpr bool empty() const { return size() == 0; }
    constexpr reference operator[](int index) const { return {this, index}; }
    constexpr iterator begin() const { return {this, 0}; }
    constexpr iterator end() const { return {this, size()}; }
    constexpr void push_back(const T& value) const {
        int n = size();
        put<T, Codec>(n, value, this);
        put<int, binary<int>>(-1, n + 1, this);
    }
    constexpr void pop_back() const { put<int, binary<int>>(-1, size() - 1, this); }
    constexpr void clear() const { put<int, binary<int>>(-1, 0, this); }
};

inline constexpr cache_vector<int> numbers;
static_assert(std::random_access_iterator<decltype(numbers.begin())>);
static_assert(std::sortable<decltype(numbers.begin())>);
static_assert([] {
    auto v = numbers;
    for (int n : {7, 2, 9, 1, 2, 0}) v.push_back(n);
    return !std::ranges::is_sorted(v);
}());
static_assert([] {
    auto v = numbers;
    std::ranges::sort(v);
    return std::ranges::is_sorted(v) && v[0] == 0 && v[5] == 9;
}());
static_assert([] {
    auto v = numbers;
    return v.size() == 6 && v[1] == 1 && v[2] == 2 && v[3] == 2 && v[4] == 7;
}());

static_assert([] {
    auto v = numbers;
    auto first = v[0];
    if (int(first) != 0) return false;
    v[0] = -4;
    if (int(first) != -4) return false; // The saved proxy is live this time.
    using std::swap;
    swap(v[0], v[5]);
    if (v[0] != 9 || v[5] != -4) return false;
    std::iter_swap(v.begin(), v.begin() + 5);
    std::ranges::swap(v[0], v[5]);
    std::ranges::iter_swap(v.begin(), v.begin() + 5);
    if (v[0] != -4 || v[5] != 9) return false;
    std::sort(v.begin(), v.end(), std::greater<>{});
    if (!std::is_sorted(v.begin(), v.end(), std::greater<>{})) return false;
    std::ranges::sort(v);
    return std::ranges::equal(v, std::array{-4, 1, 2, 2, 7, 9});
}());

inline constexpr cache_vector<std::string, text> words;
static_assert(std::sortable<decltype(words.begin())>);
static_assert([] {
    auto v = words;
    v.push_back(std::string(80, 'z'));
    v.push_back("apple");
    v.push_back(std::string("a\0b", 3));
    v.push_back("");
    std::ranges::sort(v);
    return std::ranges::is_sorted(v) && std::string(v[0]).empty()
        && std::string(v[1]) == std::string("a\0b", 3)
        && std::string(v[2]) == "apple" && std::string(v[3]) == std::string(80, 'z');
}());
static_assert([] {
    auto v = words;
    if (!std::ranges::is_sorted(v) || std::string(v[3]).size() != 80) return false;
    v.clear();
    if (!std::ranges::is_sorted(v)) return false;
    v.push_back("singleton");
    if (!std::ranges::is_sorted(v)) return false;
    v.pop_back();
    return v.empty();
}());

// A field-wise codec, not a byte copy of this padded object.
struct record {
    char label;
    int rank;
    constexpr record(char c, int n) : label(c), rank(n) {}
};
static_assert(!std::is_default_constructible_v<record>);
struct record_codec {
    static constexpr bytes encode(const record& r) {
        auto b = binary<int>::encode(r.rank);
        b.push_back(static_cast<unsigned char>(r.label));
        return b;
    }
    static constexpr record decode(bytes b) {
        char label = static_cast<char>(b.back());
        b.pop_back();
        return {label, binary<int>::decode(b)};
    }
};
inline constexpr cache_vector<record, record_codec> records;
static_assert([] {
    auto v = records;
    v.push_back({'c', 3});
    v.push_back({'a', 1});
    v.push_back({'b', 2});
    auto less = [](record a, record b) { return a.rank < b.rank; };
    std::ranges::sort(v, less);
    return std::ranges::is_sorted(v, less) && record(v[0]).label == 'a'
        && record(v[1]).label == 'b' && record(v[2]).label == 'c';
}());
static_assert([] {
    auto v = records;
    return record(v[0]).rank == 1 && record(v[2]).rank == 3;
}());
static_assert(std::is_empty_v<decltype(numbers)> && sizeof(numbers) == 1);
static_assert(std::is_empty_v<decltype(words)> && sizeof(words) == 1);

// More than libstdc++'s small-range insertion-sort threshold.
struct partition_test;
static_assert([] {
    cache_vector<int, binary<int>, partition_test> v;
    std::array<int, 20> expected{};
    for (int i = 0; i < 20; ++i) v.push_back(expected[i] = (i * 7) % 11 - 5);
    std::ranges::sort(expected);
    std::ranges::sort(v);
    return std::ranges::equal(v, expected);
}());
