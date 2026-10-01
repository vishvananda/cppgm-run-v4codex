using I=int __attribute__((ext_vector_type(4)));
#if !__has_builtin(__builtin_convertvector) || !__has_builtin(__builtin_reduce_or) || !__has_builtin(__builtin_bit_cast)
#error registry probes
#endif
int calls;
I source(){++calls;return I{1,2,3,4};}
unsigned repr(float x) noexcept {return __builtin_bit_cast(unsigned,x);}
int simple(I x) noexcept {return ::__builtin_reduce_or(x);}
static_assert(noexcept(__builtin_bit_cast(unsigned,1.0f)),"typed operation");
static_assert(!noexcept(__builtin_reduce_or(source())),"operand may throw");
int main(){return __builtin_reduce_or(__builtin_convertvector(source(),I))==7 && calls==1 && repr(1.f)==0x3f800000 && __builtin_bit_cast(unsigned,(++calls,1.f))==0x3f800000 && calls==2 ?0:1;}
