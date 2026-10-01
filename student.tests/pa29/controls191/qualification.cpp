using V=int __attribute__((ext_vector_type(4)));
int main(){V x{1,2,3,4}; return ::__builtin_bit_cast(unsigned,1.f)==0x3f800000 &&
 ::__builtin_reduce_or(::__builtin_convertvector(x,V))==7 ? 0:1;}
