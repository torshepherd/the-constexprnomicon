// One pointer; the compiler remembers the rest.
// Constant-evaluation sketch for trivially copyable/destructible T.
// Keep the vector itself uncopied; pop() expects an element.

template<class T>
struct vector {
    union slot { char empty = 0; T value; };
    slot* p = nullptr;

    constexpr ~vector() { delete[] p; }

    consteval int size() const {
        int n = 0;
        while (p && !__builtin_constant_p(p[n].empty)) ++n;
        return n;
    }
    consteval int capacity() const {
        int n = size();
        while (__builtin_constant_p(p[n].empty)) ++n;
        return n - !!p;
    }
    consteval T& operator[](int i) { return p[i].value; }
    consteval void push(T x) {
        int n = size();
        if (n == capacity()) {
            auto q = new slot[2 * n + 2];
            for (int i = n; i--;) q[i] = p[i];
            delete[] p;
            p = q;
        }
        p[n] = {.value = x};
    }
    consteval T pop() {
        int n = size() - 1;
        T x = p[n].value;
        return p[n].empty = 0, x;
    }
};

static_assert(sizeof(vector<int>) == sizeof(void*));
static_assert(sizeof(vector<int>::slot) == sizeof(int));

static_assert([] {
    vector<int> v;
    if (v.size() || v.capacity()) return false;
    for (int i = 0; i != 5; v.push(i++)) {}
    if (v.size() != 5 || v.capacity() != 7) return false;
    if (v.pop() != 4 || v.pop() != 3) return false;
    if (v.size() != 3 || v.capacity() != 7) return false;
    v[0] = 42;
    v.push(-2147483647 - 1);
    v.push(2147483647);
    if (v.pop() != 2147483647 || v.pop() != (-2147483647 - 1)) return false;
    while (v.size()) v.pop();
    if (v.capacity() != 7) return false;
    v.push(10);
    return v.size() == 1 && v[0] == 10;
}());

static_assert([] {
    vector<double> v;
    v.push(0.0); v.push(1.5); v.push(-2.25);
    return v.size() == 3 && v.capacity() == 3 && v.pop() == -2.25
        && v.size() == 2 && v.capacity() == 3;
}());

static_assert([] {
    vector<bool> v;
    v.push(false); v.push(true);
    return v.size() == 2 && v.pop() && !v.pop() && !v.size();
}());

struct point { int x, y; };
static_assert([] {
    vector<point> v;
    v.push({1, 2}); v.push({3, 4}); v.push({0, 0}); v.push({5, 6});
    if (v.size() != 4 || v.capacity() != 7) return false;
    if (v.pop().y != 6 || v.pop().x != 0) return false;
    v[0].x = 7;
    return v.size() == 2 && v[0].x == 7 && v[1].y == 4;
}());
static_assert([] {
    int x = 42, y = 13;
    vector<int*> v;
    v.push(&x); v.push(nullptr); v.push(&y);
    return v.size() == 3 && *v.pop() == 13 && !v.pop() && *v.pop() == 42;
}());
struct number {
    int n;
    constexpr number(int x) : n(x) {}
};
static_assert([] {
    vector<number> v;
    v.push(42); v.push(13);
    return v.size() == 2 && v.pop().n == 13 && v.pop().n == 42;
}());
