template<class T,T... I> struct Pack {};
template<class T,T... I> int count(Pack<T,I...>) { return sizeof...(I); }
template int count<int,1,2>(Pack<int,1,2>);
