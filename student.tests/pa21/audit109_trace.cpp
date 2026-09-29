namespace std {
class type_info {
public:
    bool operator==(const type_info&) const;
};
template<class T> class initializer_list {
    const T* data;
    unsigned long count;
public:
    const T* begin() const { return data; }
    const T* end() const { return data+count; }
};
}
int live;
struct Item {
    int value;
    Item(int n): value(n) { ++live; }
    Item(const Item& x): value(x.value) { ++live; }
    ~Item() { --live; }
};
struct Base { virtual int value() { return 1; } };
struct Derived: Base { int value() { return 2; } };
template<int N> int inspect(Base* p, int n) {
    std::initializer_list<Item> values{n,2};
    auto closure = [values,p]() {
        int sum = 0;
        for (const auto& item: values) sum += item.value;
        return sum + (dynamic_cast<Derived*>(p) ? 3 : 0)
                   + (typeid(*p)==typeid(Derived) ? 5 : 0);
    };
    for (int i=0; i<2; ++i) {
        try {
            if (!i) throw N;
            return closure();
        } catch (int value) { if (value==N) continue; }
    }
    return -1;
}
int main() {
    Base b; Derived d;
    if (inspect<0>(&b,1)!=3 || inspect<1>(&d,4)!=14 || inspect<0>(&b,3)!=5) return 1;
    return live;
}
