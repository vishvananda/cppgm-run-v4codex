struct choice {
  constexpr operator int() const { return 1; }
  constexpr operator double() const { return 1; }
};
static_assert(choice{}, "ambiguous conversion");
