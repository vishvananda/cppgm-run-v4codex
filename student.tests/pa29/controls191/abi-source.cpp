#include "abi.h"
extern "C" C nc(C x){return x;}
extern "C" B nb(B x){return x;}
extern "C" S ns(S x){return x;}
extern "C" I ni(I x){return x;}
int native_test(){C c{3}; B b{true,false,true,false}; S s{1,2,3,4}; I i{5,6,7,8};
 return __builtin_reduce_or(hc(c)!=c)==0 && __builtin_reduce_or(hb(b)) &&
 __builtin_reduce_or(hs(s)!=s)==0 && __builtin_reduce_or(hi(i)!=i)==0;}
extern "C" W nw(W x){return x;}
extern "C" X nx(X x){return x;}
extern "C" Y ny(Y x){return x;}
int native_wide(){W w{1,2,3,4,5,6,7,8};X x{9,10,11};Y y{17,18,19};
 return __builtin_reduce_or(hw(w)!=w)==0 && __builtin_reduce_or(hx(x)!=x)==0 && __builtin_reduce_or(hy(y)!=y)==0;}
