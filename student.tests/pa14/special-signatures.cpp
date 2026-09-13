// N3485 [class.conv.fct]: distinct conversion targets name distinct functions;
// their exception specifications need not match each other.
int destroyed;
template<class T> struct Maker {
    int value;
    Maker(int);
    Maker(double);
    Maker(long);
    ~Maker();
    operator int() const noexcept;
    operator double() const;
};
template<class U> Maker<U>::Maker(int x) : value(x) {}
template<class U> Maker<U>::Maker(double x) : value(static_cast<int>(x)+10) {}
template<class U> Maker<U>::Maker(long) { typename U::missing_type unused; }
template<class U> Maker<U>::~Maker() { destroyed+=value; }
template<class U> Maker<U>::operator int() const noexcept { return value; }
template<class U> Maker<U>::operator double() const { return value+0.5; }
int main() {
    {
        Maker<int> a(3); Maker<long> b(4.0);
        if (static_cast<int>(a)!=3 || static_cast<int>(b)!=14) return 1;
        if (static_cast<double>(a)!=3.5 || static_cast<double>(b)!=14.5) return 2;
    }
    return destroyed!=17;
}
