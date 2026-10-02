typedef int Ints __attribute__((vector_size(8)));
Ints f(long long x) { return static_cast<Ints>(x); }
