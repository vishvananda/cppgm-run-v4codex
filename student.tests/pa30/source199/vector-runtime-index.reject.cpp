typedef short Shorts __attribute__((vector_size(8)));
int f(Shorts v,int i) { return __builtin_ia32_vec_ext_v4hi(v,i); }
