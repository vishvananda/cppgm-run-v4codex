typedef int Ints __attribute__((vector_size(8)));
Ints f(int x) { return (Ints)x; }
