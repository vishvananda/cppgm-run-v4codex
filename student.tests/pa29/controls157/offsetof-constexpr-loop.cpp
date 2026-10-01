struct A { int x[8]; };
constexpr unsigned long offsets() {unsigned long n=0; for(int i=0;i<4;++i)n+=__builtin_offsetof(A,x[i]);return n;}
static_assert(offsets()==24,"index is re-evaluated at each constexpr execution");
int main(){return offsets()!=24;}
