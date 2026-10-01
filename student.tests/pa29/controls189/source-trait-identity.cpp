template<bool B> struct flag { static const bool value = B; };
template<class F, class A> struct property : flag<false> {};
template<class T> struct invert : flag<!T::value> {};
template<class V, class H> struct cache : invert<property<const H&, const V&>> {};
template<class V> struct hash { int operator()(V) const noexcept { return 0; } };
template<class V> struct throwing_hash { int operator()(V) const { return 0; } };
template<class V> struct property<const throwing_hash<V>&, const V&> : flag<true> {};
// Only the declared primary/specialization decides these answers.
static_assert(cache<int, hash<int>>::value, "no invented invocation trait");
static_assert(cache<int*, hash<int*>>::value, "second primary specialization");
static_assert(!cache<int, throwing_hash<int>>::value, "source specialization");
static_assert(!cache<int*, throwing_hash<int*>>::value, "second specialization");
int main() { return cache<int, hash<int>>::value && !cache<int, throwing_hash<int>>::value ? 0 : 1; }
