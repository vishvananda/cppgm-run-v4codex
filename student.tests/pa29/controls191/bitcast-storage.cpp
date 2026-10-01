struct Pair{unsigned a,b;};
unsigned long long f(Pair x){return __builtin_bit_cast(unsigned long long,x);}
Pair g(unsigned long long x){return __builtin_bit_cast(Pair,x);}
int main(){Pair p{3,7};auto x=f(p);auto q=g(x); return q.a==3 && q.b==7 && x==0x700000003ULL ? 0:1;}
