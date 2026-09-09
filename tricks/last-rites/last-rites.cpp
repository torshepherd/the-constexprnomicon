// The semicolon runs backpropagation. Keep each graph in one full expression.
struct number {
    double value;
    mutable double gradient = 0;
    const number *left, *right;
    double dl, dr;

    constexpr number(double v, const number* l = nullptr, double dl = 0,
                              const number* r = nullptr, double dr = 0)
        : value(v), left(l), right(r), dl(dl), dr(dr) {}
    number(const number&) = delete;
    number& operator=(const number&) = delete;

    constexpr ~number() {
        if (left) left->gradient += gradient * dl;
        if (right) right->gradient += gradient * dr;
    }

    constexpr double backward() && { gradient = 1; return value; }

    friend constexpr number operator+(const number& a, const number& b) {
        return {a.value + b.value, &a, 1, &b, 1};
    }
    friend constexpr number operator*(const number& a, const number& b) {
        return {a.value * b.value, &a, b.value, &b, a.value};
    }
};

static_assert([] {
    number x{3}, y{4};
    double result = (x*x + x*y).backward();
    return result == 21 && x.gradient == 10 && y.gradient == 3;
}());

static_assert([] {
    number x{3};
    (x*x*x).backward();
    return x.gradient == 27;
}());

static_assert([] {
    number x{2}, y{3};
    ((x+y)*(x+y)).backward();
    return x.gradient == 10 && y.gradient == 10;
}());

static_assert([] {
    number x{3}, y{4};
    (2*x*x + 3*x*y + y*y + 7).backward();
    return x.gradient == 24 && y.gradient == 17;
}());

static_assert([] {
    number x{0}, y{-2};
    (x*x*y + 3*x + y*y).backward();
    return x.gradient == 3 && y.gradient == -4;
}());

static_assert([] {
    number x{3};
    (x*x).backward();
    (2*x).backward();
    return x.gradient == 8;
}());

static_assert([] {
    number x{3};
    (void)(x*x + 2*x);
    return x.gradient == 0;
}());

static_assert([] {
    number x{3};
    bool seeded_before_cleanup = ((x*x).backward(), x.gradient == 0);
    return seeded_before_cleanup && x.gradient == 6;
}());
