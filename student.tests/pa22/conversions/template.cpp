struct Pad { int pad; };
struct Value {
    int x;
    template<class T> operator T() const { return T(x); }
};
struct Other { explicit operator bool() const { return false; } };
struct D : Pad, Other, Value {};
template<class T> auto convert(const T& t) -> decltype(int(t)) { return int(t); }
struct E : D { operator int() const { return 17; } };
int main() {
    D d; d.x = 9;
    E e; e.x = 11;
    return convert(d) != 9 || bool(d) || short(d) != 9 || convert(e) != 17 || short(e) != 11;
}
