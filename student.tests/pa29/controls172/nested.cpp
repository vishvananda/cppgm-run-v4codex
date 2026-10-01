template<int... N> struct Pack {
 template<int... M> static int sum() {return ((N + (M + ... + 0)) + ... + 0);}
};
template<class... T> int size_sum(T...x) {return ((sizeof...(T)+sizeof(x)) + ... + 0);}
int main(){return Pack<1,2,3>::sum<4,5>()!=33 || size_sum(1,2)!=12;}
