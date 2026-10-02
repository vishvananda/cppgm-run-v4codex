typedef int V __attribute__((vector_size(16)));
template<class T> V change(T v) {V out={};out=v;out+=V{1,2,3,4};return ~(-out);}
static_assert(__is_same(decltype(~V{}),V),"query unary vector");
int main(){V v=change(V{1,2,3,4});return v[0]!=1||v[3]!=7;}
