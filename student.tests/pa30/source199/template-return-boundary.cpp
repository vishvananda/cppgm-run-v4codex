template<class T> struct Result { T value; };
template<class T> struct Service {
    template<class R> R first(R value) const { return value; }
    template<class U> Result<U> second(U value) const { return Result<U>{value}; }
    template<class V> Result<V> third(V value) const volatile { return Result<V>{value}; }
};
int main() { Service<int> s; return s.first(7)+s.second(9).value+s.third(11).value-27; }
