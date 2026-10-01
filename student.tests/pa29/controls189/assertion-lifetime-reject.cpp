struct choice {
  constexpr explicit operator bool() const { return true; }
  ~choice() {}
};
static_assert(choice{}, "nonliteral receiver");
