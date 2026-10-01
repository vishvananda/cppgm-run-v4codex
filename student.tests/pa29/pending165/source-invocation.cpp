extern "C" int printf(const char*,...);
constexpr int inner(int n=__builtin_LINE()){return n;}
constexpr int outer(int n=inner()){return n;}
struct S{int n=__builtin_LINE(); S(){} };
int main(){printf("%d %d %d\n",inner(),outer(),S().n);}
