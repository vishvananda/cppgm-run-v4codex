class choice { constexpr explicit operator bool() const { return true; } };
static_assert(choice{}, "private conversion");
