template<unsigned I,class... T> __type_pack_element<I,T...> abi_pick(T...);
template<class T,T... I> struct seq {};
template<int N> __make_integer_seq<seq,int,N> abi_seq();
int main(){abi_seq<3>();return abi_pick<1>('a',0);}
