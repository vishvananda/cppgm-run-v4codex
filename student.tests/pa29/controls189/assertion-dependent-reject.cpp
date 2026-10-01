template<bool B> struct choice { constexpr explicit operator bool() const { return B; } };
template<bool B> struct check { static_assert(choice<B>{}, "dependent false"); };
check<false> instantiated;
