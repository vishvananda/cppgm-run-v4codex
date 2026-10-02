// Unfinished inherited vector-expression surface, independently of fixed
// construction/extraction builtins. The entry compiler rejects this too.
typedef int Vector __attribute__((vector_size(8)));
int main() { Vector value = {3,4}; return value[1]-4; }
