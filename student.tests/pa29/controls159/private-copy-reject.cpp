struct Private { private: Private(const Private&) noexcept; };
static_assert(__has_nothrow_copy(Private),"legacy property");
Private copy(const Private& p){return p;}
