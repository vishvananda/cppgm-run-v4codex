constexpr int expired() { int *p=0; if (int n=3; n) p=&n; return *p; }
static_assert(expired()==3,"object outlives selection");
