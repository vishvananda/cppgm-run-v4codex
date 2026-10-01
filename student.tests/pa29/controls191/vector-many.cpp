using I=int __attribute__((ext_vector_type(4)));
using F=float __attribute__((ext_vector_type(4)));
using B=bool __attribute__((ext_vector_type(4)));
I copy(I v) noexcept{return v;}
int main(int argc,char**) {
 I a{argc,2,3,4}, b{1,2,3,4};
 I x=copy(a); F f=__builtin_convertvector(a,F); I back=__builtin_convertvector(f,I);
 B mask=__builtin_convertvector(a==b,B);
 return __builtin_reduce_or(x!=b)==0 && __builtin_reduce_or(back!=b)==0 &&
 __builtin_reduce_or(mask) && __builtin_reduce_or(a)==7 ? 0:1;
}
