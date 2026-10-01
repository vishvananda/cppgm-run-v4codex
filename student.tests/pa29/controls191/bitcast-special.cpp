constexpr unsigned roundtrip(unsigned x){return __builtin_bit_cast(unsigned,__builtin_bit_cast(float,x));}
constexpr unsigned long long wide(unsigned long long x){return __builtin_bit_cast(unsigned long long,__builtin_bit_cast(double,x));}
static_assert(roundtrip(0x80000000)==0x80000000,"negative zero");
static_assert(roundtrip(0x7fc12345)==0x7fc12345,"quiet nan payload");
static_assert(roundtrip(0x7f812345)==0x7f812345,"signaling nan payload");
static_assert(wide(0xfff0102030405060ULL)==0xfff0102030405060ULL,"wide signaling nan payload");
int main(){unsigned x=0x7f812345;float f=__builtin_bit_cast(float,x);return __builtin_bit_cast(unsigned,f)==x?0:1;}
