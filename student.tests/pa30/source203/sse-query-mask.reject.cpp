typedef float F __attribute__((vector_size(16)));
template<int M> struct Result {using type=decltype(__builtin_ia32_shufps(F{},F{},M));};
Result<256>::type bad;
