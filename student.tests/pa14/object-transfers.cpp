// Prepared transfer actions preserve bindings, base identity and source effects.
int calls = 0, moves = 0, copies = 0;
struct Empty {};
Empty empty;
Empty& source_empty() { ++calls; return empty; }
struct Holder {
    Empty member;
    Holder() : member(source_empty()) {}
};
template<class T> struct Binding {
    T* pointer;
    T& reference;
    Binding(T* p, T& r) : pointer(p), reference(r) {}
    Binding(Binding&&) = default;
};
template<class T> struct Outer {
    struct Reference {
        T& value;
        Reference(T& x) : value(x) {}
        Reference(const Reference&);
    };
};
template<class T> Outer<T>::Reference::Reference(const Reference&) = default;
int read(Outer<int>::Reference x) { return x.value; }
template<class T> T&& move_from(T& x) { return static_cast<T&&>(x); }
struct Value {
    int number;
    Value(int n) : number(n) {}
    Value(const Value& x) : number(x.number) { ++copies; }
    Value(Value&& x) : number(x.number) { ++moves; x.number = 0; }
    ~Value() {}
};
Value produce(Value& source) { Value result = move_from(source); return result; }
template<class T, class Base = Empty> struct Chain : Base {
    T number;
    Chain(T n) : Base(), number(n) {}
    Chain(T n, const Base& b) : Base(b), number(n) {}
    Chain(const Chain& x) : Base(x), number(x.number + 1) {}
};
namespace layout {
struct Other {};
template<class From, class To> struct Compatible {
    static char test(To);
    static int test(...);
    enum { result = sizeof(test((From)0)) };
};
template<class T> struct Link : Compatible<Other*, T*> { int value; };
struct Node;
struct Node { int prefix[6]; Link<Node> link; int tail; };
int extent() { return sizeof(Link<Node>); }
}
int main() {
    Holder holder;
    if (calls != 1) return 1;
    int n = 9;
    Binding<int> a(&n, n);
    Binding<int> b(static_cast<Binding<int>&&>(a));
    if (b.pointer != &n || &b.reference != &n) return 2;
    Outer<int>::Reference first(n), second(first);
    second.value = 13;
    if (&second.value != &n || read(first) != 13) return 3;
    Value initial(7);
    Value result = produce(initial);
    if (result.number != 7 || initial.number != 0 || moves != 1 || copies != 0) return 4;
    Chain<int> inner(3);
    Chain<long, Chain<int>> outer(5, inner);
    if (outer.number != 5 || static_cast<Chain<int>&>(outer).number != 4) return 5;
    if (layout::extent() != 4 || sizeof(layout::Node) != 32) return 6;
    return 0;
}
