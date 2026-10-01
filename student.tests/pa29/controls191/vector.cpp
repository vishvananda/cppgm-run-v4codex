using size_t=decltype(sizeof(0));
template<class T,size_t N> using V __attribute__((ext_vector_type(N)))=T;
template<class T,size_t N> bool any(V<T,N> x) noexcept {
 return __builtin_reduce_or(__builtin_convertvector(x,V<bool,N>));
}
template<class T> T bits(bool x){return __builtin_bit_cast(T,x);}
int main(){ V<char,1> a{},b{}; auto x=a==b;
return any(x) && bits<unsigned char>(true)==1 ? 0:1; }
