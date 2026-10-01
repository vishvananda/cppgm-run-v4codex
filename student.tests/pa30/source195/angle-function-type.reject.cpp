template<class T> struct wrap { constexpr operator bool() const { return true; } };
template<bool B> struct value_only {};
value_only<wrap<wrap<int>>()> invalid;
