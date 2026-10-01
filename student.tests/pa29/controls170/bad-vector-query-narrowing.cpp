typedef int V __attribute__((vector_size(16)));
template<class T> auto f()->decltype(T{1.5});
using R=decltype(f<V>());
