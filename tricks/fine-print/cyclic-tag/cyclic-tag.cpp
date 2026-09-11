// Cyclic tag computation in noexcept. C++11; no function definitions.
// Binary queues use three overloads. Y/N add observable Boolean halts.
template<char...> struct word {};
template<class...> struct cycle {};

template<class... Rules> void run(cycle<Rules...>, word<>) noexcept;

template<char... Append, class... Rest, char... Tail>
void run(cycle<word<Append...>, Rest...>, word<'0', Tail...>)
    noexcept(noexcept(run(cycle<Rest..., word<Append...>>{}, word<Tail...>{})));

template<char... Append, class... Rest, char... Tail>
void run(cycle<word<Append...>, Rest...>, word<'1', Tail...>)
    noexcept(noexcept(run(cycle<Rest..., word<Append...>>{}, word<Tail..., Append...>{})));

template<class... Rules, char... Tail>
void run(cycle<Rules...>, word<'Y', Tail...>) noexcept;
template<class... Rules, char... Tail>
void run(cycle<Rules...>, word<'N', Tail...>) noexcept(false);

using program = cycle<word<'1','1'>, word<>, word<>>;
static_assert(noexcept(run(program{}, word<'1'>{})), "grow, rotate twice, halt");
static_assert(noexcept(run(program{}, word<'1','1'>{})), "reuse rules across cycles");
static_assert(noexcept(run(program{}, word<'0','0','0'>{})), "zero consumes without appending");

using answer = cycle<word<'Y'>, word<'N'>>;
static_assert(noexcept(run(answer{}, word<'1'>{})), "first rule appends yes");
static_assert(!noexcept(run(answer{}, word<'0','1'>{})), "second rule appends no");
static_assert(noexcept(run(answer{}, word<>{})), "empty binary tape halts");
