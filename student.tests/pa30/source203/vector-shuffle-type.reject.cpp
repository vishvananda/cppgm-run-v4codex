typedef int I __attribute__((vector_size(16)));
typedef float F __attribute__((vector_size(16)));
I f(I a,F b) {return __builtin_shuffle(a,b,I{});}
