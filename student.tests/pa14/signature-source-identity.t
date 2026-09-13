// N3485 [temp.inst]/1: earlier class instantiation does not change the
// source/prototype identity of a subsequently declared member signature.
template<class T> struct Early { T value; };
Early<int> early;
template<class T> struct Later {
    T add(T first, decltype(first) second) { return first+second; }
    auto identity(T& value) -> decltype((value)) { return value; }
};
int main() {
    Later<int> a; Later<long> b; int value=4;
    return a.add(2,3)!=5 || b.add(3,4)!=7 || &a.identity(value)!=&value;
}
