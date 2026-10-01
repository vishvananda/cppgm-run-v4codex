using I=int __attribute__((ext_vector_type(4)));using J=int __attribute__((ext_vector_type(2)));J f(I x){return __builtin_convertvector(x,J);}
