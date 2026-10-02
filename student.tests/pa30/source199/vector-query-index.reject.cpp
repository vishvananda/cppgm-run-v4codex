typedef int Ints __attribute__((vector_size(8)));
using Bad=decltype(__builtin_ia32_vec_ext_v2si(Ints{},-1));
