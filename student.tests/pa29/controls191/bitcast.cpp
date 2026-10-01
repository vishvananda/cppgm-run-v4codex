static_assert(__builtin_bit_cast(unsigned,1.0f)==0x3f800000,"float representation");
template<class T> constexpr unsigned bits(T x){return __builtin_bit_cast(unsigned,x);}
static_assert(bits(2.0f)==0x40000000,"template");
int main(int argc,char**){float x=float(argc); auto y=__builtin_bit_cast(unsigned,x);return y==0x3f800000?0:1;}
