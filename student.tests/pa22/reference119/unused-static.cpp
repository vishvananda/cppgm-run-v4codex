template<bool B> struct Flag {
  static const bool value=B;
  constexpr operator bool() const { return B; }
};
template<bool B> const bool Flag<B>::value;
int main() { return Flag<false>() ? 1 : 0; }
