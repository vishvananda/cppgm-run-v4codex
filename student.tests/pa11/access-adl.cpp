namespace domain {
class Value {
    int data;
    friend int read(const Value&);
    friend int hidden(const Value& v) { return v.data + 1; }
public:
    Value(int n) : data(n) {}
};
int read(const Value& v) { return v.data; }
}
struct Base {
protected:
    int data;
public:
    Base() : data(7) {}
    int select(int) { return 1; }
};
class Derived : private Base {
    friend struct Inspector;
public:
    using Base::select;
    int select(int) { return 2; }
    static const Base* convert(const Derived* d) { return d; }
};
struct Inspector {
    static int read(Derived& d) { return d.data; }
};
namespace enum_domain {
enum Tag { chosen = 3 };
int inspect(Tag t) { return int(t); }
}
int main() {
    Derived d;
    if (Inspector::read(d) != 7 || d.select(0) != 2) return 1;
    if (Derived::convert(&d) == nullptr) return 2;
    if (read(domain::Value(4)) != 4 || hidden(domain::Value(8)) != 9) return 3;
    return inspect(enum_domain::chosen) - 3;
}
