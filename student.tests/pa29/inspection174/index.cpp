template<class... T> __type_pack_element<0,T...> first(T... values) { return (... + values); }
template int first<int,int>(int,int);
