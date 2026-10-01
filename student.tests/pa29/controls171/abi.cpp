template<unsigned I,class... T> __type_pack_element<I,T...> abi_pick(T...) { return {}; }
template int abi_pick<1,char,int>(char,int);
template<class T,T... I> struct seq {};
template<int N> __make_integer_seq<seq,int,N> abi_seq(){return {};}
template seq<int,0,1,2> abi_seq<3>();
#ifndef ABI_PEER
int main(){return abi_pick<1>('a',0);}
#endif
