struct S{int n;~S(){}};int f(S x){return __builtin_bit_cast(int,x);}
