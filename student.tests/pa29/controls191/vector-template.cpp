using I=int __attribute__((ext_vector_type(4)));
using B=bool __attribute__((ext_vector_type(9)));
template<int N> I step(I x){I c{N&127,(N*2)&127,(N*3)&127,(N*4)&127};return x^c;}
template<int N> I copy(I x){I y=x; return y;}
template<int N> I fixed(){return I{1,2,3,4};}
template<int N> I dependent(){return {N,2,3,4};}
template<int N> struct Store { I value{N,2,3,4}; B mask{true,false,true,false,false,false,false,true,true}; };
int main(){I x{1,2,3,4};Store<1> s;
 return __builtin_reduce_or(step<3>(x)^I{2,4,10,8})==0 &&
 __builtin_reduce_or(copy<1>(x)^fixed<1>())==0 &&
 __builtin_reduce_or(dependent<1>()^s.value)==0 && __builtin_bit_cast(unsigned short,s.mask)==389 ? 0:1;
}
