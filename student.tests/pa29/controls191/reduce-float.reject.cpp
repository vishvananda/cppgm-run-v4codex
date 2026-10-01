using F=float __attribute__((ext_vector_type(4)));float f(F x){return __builtin_reduce_or(x);}
