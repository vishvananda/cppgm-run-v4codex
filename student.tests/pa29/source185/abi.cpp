#include "../support185/abi-types.h"
U93 student_stack(long a,long b,long c,long d,long e,long f,long g,U93 x,long y) { return x+a+b+c+d+e+f+g+y; }
Payload student_object(Payload x) { x.wide+=3; x.tail+=1; return x; }
S7 student_small(S7 x) { return x-2; }
U93 student_variadic(int tag,...) {
 __builtin_va_list ap; __builtin_va_start(ap,tag);
 long sum=0; for(int i=0;i<6;++i) sum+=__builtin_va_arg(ap,long);
 U93 x=__builtin_va_arg(ap,U93); long last=__builtin_va_arg(ap,long);
 __builtin_va_end(ap); return x+sum+last+tag;
}
int main() {
 U93 x=U93(1)<<85;
 if(host_stack(1,2,3,4,5,6,7,x,8)!=x+36) return 1;
 if(host_small(S7(-3))!=-5) return 2;
 Payload p={1,x,2}; p=host_object(p);
 if(p.lead!=1 || p.wide!=x+3 || p.tail!=3) return 3;
 if(host_variadic(7,1L,2L,3L,4L,5L,6L,x,8L)!=x+36) return 4;
 return host_drive();
}
