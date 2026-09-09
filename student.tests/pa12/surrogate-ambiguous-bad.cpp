typedef int (*First)(int,double);
typedef int (*Second)(double,int);
struct Callable { operator First(); operator Second(); };
int main() { Callable f; return f(1,2); }
