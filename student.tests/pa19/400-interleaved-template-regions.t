// N3485 focus: [temp.inst], [temp.variadic], [dcl.fct.default].
// Each specialization first exposes declarations, then independently demands
// nested-class bodies, defaults and differently sized pack expansions.
template<class T> struct Box {
    T value;
    struct Nested { static T twice(T v) { return v + v; } };
    T add(T amount = T(3)) const { return value + amount; }
    template<class... U> int count(U... values) const {
        int items[] = {int(value), int(values)...};
        return items[0] + sizeof...(values);
    }
    int unused() { return T::invalid_member; }
};

int main()
{
    Box<int> a{5};
    Box<long> b{9};
    int x = a.count(1, 2);
    int y = b.count();
    int z = a.count(4);
    return x != 7 || y != 9 || z != 6 ||
        Box<long>::Nested::twice(b.add()) != 24 ||
        Box<int>::Nested::twice(a.add()) != 16 || b.add(2) != 11;
}
