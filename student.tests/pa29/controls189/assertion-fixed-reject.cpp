struct choice { constexpr explicit operator bool() const { return false; } };
template<class T> struct unused { static_assert(choice{}, "fixed false"); };
unused<int> instantiated;
