template<class... T> struct Bag {
    static const int value = sizeof...(T);
    typedef int type;
};
template<class T> auto query() -> decltype(Bag<T,T>::value) { return 2; }
template const int query<int>();
template<class T> typename Bag<T,T>::type member() { return 2; }
template int member<int>();
