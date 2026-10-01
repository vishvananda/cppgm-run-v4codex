using I=int __attribute__((ext_vector_type(4)));int f(I x){return __builtin_reduce_or(x,x);}
