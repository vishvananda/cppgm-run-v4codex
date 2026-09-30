template<class T> struct Box {
    typedef T value_type;
    value_type get();
    value_type other(void);
    template<class U> value_type nested(void);
};
template<class U> typename Box<U>::value_type Box<U>::get(void) { return 17; }
template<class U> typename Box<U>::value_type Box<U>::other() { return 19; }
template<class U> template<class V> typename Box<U>::value_type Box<U>::nested() { return sizeof(V); }
int main() { Box<int> b; return b.get()+b.other()+b.nested<long>()-44; }
