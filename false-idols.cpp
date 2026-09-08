// GCC 13.3/16.2 and Clang 22.1.0, C++20. Every atom is true.
// Constraint subsumption solves SAT; overload ambiguity reveals the answer.
template<class> concept X  = true;
template<class> concept NX = true;
template<class> concept Y  = true;
template<class> concept NY = true;

template<class T> concept contradiction =
    (X<T> && NX<T>) || (Y<T> && NY<T>);

template<class T> concept formula =
    ( X<T> ||  Y<T>) && ( X<T> || NY<T>) &&
    (NX<T> ||  Y<T>) && (NX<T> || NY<T>);

template<class T> struct oracle {
    void solve() requires contradiction<T>;
    void solve() requires (formula<T> && true);
};

template<class T> concept satisfiable =
    !requires(oracle<T> o) { o.solve(); };

static_assert(formula<int> && contradiction<int>);
static_assert(!satisfiable<int>);

// Remove the last clause: x = y = true is now a solution.
template<class T> concept relaxed =
    (X<T> || Y<T>) && (X<T> || NY<T>) && (NX<T> || Y<T>);

template<class T> struct control {
    void solve() requires contradiction<T>;
    void solve() requires (relaxed<T> && true);
};

template<class T> concept control_satisfiable =
    !requires(control<T> o) { o.solve(); };

static_assert(relaxed<int> && contradiction<int>);
static_assert(control_satisfiable<int>);
