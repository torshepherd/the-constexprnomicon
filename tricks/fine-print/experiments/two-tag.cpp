// Earlier two-tag probe, retained as research history.
// Delete two symbols; append according to the first; H is an accepting halt.
template<char...> struct word {};
void tag(...) noexcept(false);
template<char... Tail> void tag(word<'H', Tail...>) noexcept;
template<char Ignored, char... Tail>
void tag(word<'a', Ignored, Tail...>)
    noexcept(noexcept(tag(word<Tail..., 'c','c','b','a','H'>{})));
template<char Ignored, char... Tail>
void tag(word<'b', Ignored, Tail...>)
    noexcept(noexcept(tag(word<Tail..., 'c','c','a'>{})));
template<char Ignored, char... Tail>
void tag(word<'c', Ignored, Tail...>)
    noexcept(noexcept(tag(word<Tail..., 'c','c'>{})));
static_assert(noexcept(tag(word<'b','a','a'>{})), "baa reaches H");
static_assert(!noexcept(tag(word<'a'>{})), "short halt");
static_assert(!noexcept(tag(word<'b'>{})), "short halt");
static_assert(noexcept(tag(word<'H'>{})), "immediate acceptance");
#ifdef CYCLE
static_assert(!noexcept(tag(word<'c','a','c'>{})), "incorrect: cac reaches ccc and cycles");
#endif
