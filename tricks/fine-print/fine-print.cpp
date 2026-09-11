// Fine print: a parser written in exception specifications. C++11.
// No function is defined or called; noexcept is the result channel.
template<char...> struct word {};

void balanced(...) noexcept(false);
void balanced(word<>, word<>) noexcept;

template<char... Input, char... Stack>
void balanced(word<'(', Input...>, word<Stack...>)
    noexcept(noexcept(balanced(word<Input...>{}, word<'(', Stack...>{})));

template<char... Input, char... Stack>
void balanced(word<')', Input...>, word<'(', Stack...>)
    noexcept(noexcept(balanced(word<Input...>{}, word<Stack...>{})));

static_assert(noexcept(balanced(word<>{}, word<>{})), "empty");
static_assert(noexcept(balanced(word<'(',')'>{}, word<>{})), "pair");
static_assert(noexcept(balanced(word<'(','(',')',')','(',')'>{}, word<>{})), "nested and adjacent");
static_assert(!noexcept(balanced(word<')','('>{}, word<>{})), "wrong order");
static_assert(!noexcept(balanced(word<'(','(',')'>{}, word<>{})), "unclosed");
static_assert(!noexcept(balanced(word<'(',')',')'>{}, word<>{})), "extra close");
static_assert(!noexcept(balanced(word<'x'>{}, word<>{})), "unknown character");
