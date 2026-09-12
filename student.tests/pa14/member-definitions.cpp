int destroyed=0;
namespace objects {
template<class T> struct Box {
    typedef T type;
    T value;
    static T total;
    struct Nested;
    Box(T);
    ~Box();
    T read() const;
    T late() const;
    int unused() { return T::missing; }
};
}
template<class Renamed> objects::Box<Renamed>::Box(Renamed n) : value(n) {}
template<class Renamed> objects::Box<Renamed>::~Box() { ++destroyed; }
template<class Renamed> Renamed objects::Box<Renamed>::read() const { return value; }
template<class Renamed> Renamed objects::Box<Renamed>::total=Renamed(9);
template<class Renamed> struct objects::Box<Renamed>::Nested {
    Renamed get() const { return Box<Renamed>::total; }
};
int use() {
    objects::Box<int> a(7);
    objects::Box<long> b(11);
    objects::Box<int>::Nested nested;
    if (a.read()!=7 || b.read()!=11 || a.late()!=8 || nested.get()!=9) return 1;
    objects::Box<int>::total=13;
    return objects::Box<long>::total!=9 || nested.get()!=13;
}
template<class Renamed> Renamed objects::Box<Renamed>::late() const { return value+1; }
template<class T> struct Unevaluated { static T missing; };
template<class T> T Unevaluated<T>::missing=T::invalid;
int main() {
    if (sizeof(Unevaluated<int>::missing)!=sizeof(int)) return 1;
    if (use()!=0 || destroyed!=2) return 2;
    return 0;
}
