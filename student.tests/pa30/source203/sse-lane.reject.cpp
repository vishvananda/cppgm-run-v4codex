typedef short Q __attribute__((vector_size(16)));
short f(Q q,int i){return __builtin_ia32_vec_ext_v8hi(q,i);}
