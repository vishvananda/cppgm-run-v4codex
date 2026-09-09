typedef int (*Function)(int);
struct Callable { explicit operator Function(); };
int main() { Callable f; return f(1); }
