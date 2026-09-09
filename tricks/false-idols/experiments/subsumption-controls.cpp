#include "../false-idols.cpp"

// Turn the same true formula into an opaque Boolean atom.
template<class T> constexpr bool opaque = formula<T>;
template<class T> struct erased {
    void solve() requires contradiction<T>;
    void solve() requires (opaque<T> && true);
};
template<class T> concept erased_ambiguous =
    !requires(erased<T> o) { o.solve(); };
static_assert(opaque<int>);
static_assert(erased_ambiguous<int>); // Lost the structural UNSAT proof.

// A fresh true atom prevents equivalent constraints from remaining tied.
template<class T> struct without_guard {
    void solve() requires contradiction<T>;
    void solve() requires ((X<T> && NX<T>) || (Y<T> && NY<T>));
};
template<class T> struct with_guard {
    void solve() requires contradiction<T>;
    void solve() requires (((X<T> && NX<T>) || (Y<T> && NY<T>)) && true);
};
template<class T> concept unguarded_ambiguous =
    !requires(without_guard<T> o) { o.solve(); };
template<class T> concept guarded_unique =
    requires(with_guard<T> o) { o.solve(); };
static_assert(unguarded_ambiguous<int>);
static_assert(guarded_unique<int>);
