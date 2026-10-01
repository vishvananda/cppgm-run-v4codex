struct choice { constexpr explicit operator int() const { return 1; } };
static_assert(choice{}, "explicit int is not contextual bool");
