#include "../cyclic-tag/cyclic-tag.cpp"
#include <type_traits>

// An unselected exception specification is not instantiated.
template<class T> void choose(T) noexcept(sizeof(typename T::missing) == 0);
void choose(int) noexcept;
static_assert(noexcept(choose(0)), "unselected invalid candidate stays asleep");
struct valid { using missing = int; };
static_assert(!noexcept(choose(valid{})), "generic has valid specializations too");

template<class...> struct list {};
template<class... T> void stop(list<T...>)
    noexcept(noexcept(stop(list<int, T...>{})));
void stop(list<>) noexcept;
void stop(list<int, char>) noexcept;
static_assert(noexcept(stop(list<>{})), "do not enter unselected growing recursion");
static_assert(noexcept(stop(list<char>{})), "generic has a terminating specialization");

// A finite false answer is still a well-formed void call expression.
static_assert(std::is_same<decltype(run(answer{}, word<'0','1'>{})), void>::value,
              "false is not failure");

#if PROBE == 1
// One rule, one bit: exact repeated configuration. No Boolean answer.
static_assert(noexcept(run(cycle<word<'1'>>{}, word<'1'>{})), "cycle");
#elif PROBE == 2
// Increasing queue: unique configurations eventually exceed template depth.
static_assert(noexcept(run(cycle<word<'1','1'>>{}, word<'1'>{})), "growth");
#elif PROBE == 3
// No rule exists for this alphabet symbol: diagnostic, not false.
static_assert(noexcept(run(program{}, word<'x'>{})), "invalid input");
#elif PROBE == 4
// Empty production ring cannot process a nonempty queue.
static_assert(noexcept(run(cycle<>{}, word<'0'>{})), "missing program");
#elif PROBE == 5
static_assert(noexcept(choose('x')), "selected invalid spec is a hard error");
#endif
