typedef int Ints __attribute__((vector_size(8)));
template<class T> int f(Ints v) { return __builtin_ia32_vec_ext_v2si(v,2); }
