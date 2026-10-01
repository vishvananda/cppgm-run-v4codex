using B=bool __attribute__((ext_vector_type(9)));
B bits{true,false,true,false,false,false,false,true,true};
using I=int __attribute__((ext_vector_type(16)));
using J=int __attribute__((ext_vector_type(64)));
int main(int argc,char**) {
 I a{argc,2,4,8}, b{1,2,4,8}; J x{},y{};
 auto z=__builtin_convertvector(a, bool __attribute__((ext_vector_type(16))));
 return __builtin_bit_cast(unsigned short,bits)==389 && __builtin_reduce_or(a==b)==-1 &&
 __builtin_reduce_or(z) && __builtin_reduce_or(x!=y)==0 ? 0:1;
}
