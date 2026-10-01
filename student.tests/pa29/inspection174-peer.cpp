extern int copies;
template<class... T> __type_pack_element<0,T...> first(T...);
int main() { return first(2,4,6) == 15 && copies == 0 ? 0 : 1; }
