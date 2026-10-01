struct choice { explicit operator bool() const { return true; } };
static_assert(choice{}, "nonconstant bool");
