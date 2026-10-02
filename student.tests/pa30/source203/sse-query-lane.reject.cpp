typedef short S __attribute__((vector_size(16)));
using invalid=decltype(__builtin_ia32_vec_ext_v8hi(S{},8));
