struct choice { constexpr explicit operator bool() const { return false; } };
static_assert(choice{}, "converted false");
