struct choice { explicit operator bool() const = delete; };
static_assert(choice{}, "deleted conversion");
