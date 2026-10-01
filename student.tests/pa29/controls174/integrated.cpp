int copies;
struct Shape { Shape() {} Shape(const Shape&) { ++copies; } };
template<class T, T... I> struct Sequence {};
template<class... T> __type_pack_element<0,T...> first(T... values) {
    Shape shape;
    auto sum = [=]<class U, U... I>(Sequence<U,I...>, T... args) {
        decltype(shape) local;
        return (... + args) + (... + I);
    };
    return sum(__make_integer_seq<Sequence,int,3>{},values...);
}
template int first<int,int,int>(int,int,int);
#ifndef ABI_PEER
int main() { return first(2,4,6) == 15 && copies == 0 ? 0 : 1; }
#endif
