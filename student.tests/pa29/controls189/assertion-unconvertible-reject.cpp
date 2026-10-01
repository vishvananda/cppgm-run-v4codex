struct choice {};
static_assert(choice{}, "no bool conversion");
