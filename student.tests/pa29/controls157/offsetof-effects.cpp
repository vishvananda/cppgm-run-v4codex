struct A{int x[8];};
int count;
int f(){++count;return 2;}
int fail(){throw 7;}
constexpr int idx(int n){return n+1;}
constexpr unsigned long off(int n){return __builtin_offsetof(A,x[idx(n)]);}
static_assert(off(1)==8 && off(3)==16,"activation-sensitive offset");
static_assert(!noexcept(__builtin_offsetof(A,x[f()])),"index effects are evaluated");
static_assert(noexcept(__builtin_offsetof(A,x[1])),"fixed layout has no effects");
int main(){(void)__builtin_offsetof(A,x[f()]);
 if(count!=1)return 1;
 try { (void)__builtin_offsetof(A,x[fail()]); return 2; } catch(int n) {return n!=7;}}
