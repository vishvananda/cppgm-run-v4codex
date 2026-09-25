// Source -> shared cast selection -> substituted conversions -> typed storage -> LowIR.
template<int N> struct Result { static const int value=N; };
struct Constant { constexpr operator int() const { return 65543; } };
template<class T> auto probe() -> Result<static_cast<const short&>(T())>;
static_assert(decltype(probe<Constant>())::value==7,"converted constant storage");
template<class T> auto narrow(T n) -> decltype(static_cast<const short&>(n),int()) {
    const short& value=static_cast<const short&>(n);
    return value;
}
struct Bits { unsigned value:4; };
template<class T> int snapshot(T& source) {
    unsigned&& value=static_cast<unsigned&&>(source.value);
    value=3;
    return source.value==7 && value==3;
}
struct First { int a; };
struct Second { int b; };
struct Derived:First,Second {};
int is_null(Second*const& p) { return p==nullptr; }
template<class T> int null_value(T&& p) { return p==nullptr; }
int effects;
struct Scalar { explicit operator double() { ++effects; return 7.; } };
template<class T> auto converted(T& value) -> decltype(static_cast<double>(value)) {
    return static_cast<double>(value);
}
int main() {
    Bits b={7}; Derived* p=nullptr; Scalar s;
    return narrow(65543)!=7 || !snapshot(b) || !is_null(p) ||
        !null_value((int*)0) || converted(s)!=7. || effects!=1;
}
