int destroyed;
struct Base {
    int value;
    explicit Base(int n) : value(n) {}
    Base(const int& n, int add) : value(n + add) {}
    ~Base() { destroyed += value; }
};
struct Derived : Base {
    using Base::Base;
    int extra = 5;
};
struct Further : Derived { using Derived::Derived; };
struct Override : Base {
    using Base::Base;
    Override(int n) : Base(n + 10) {}
};
struct Earlier : Base {
    Earlier(int n) : Base(n + 20) {}
    using Base::Base;
};
int main() {
    int n = 3;
    {
        Derived d(4);
        Further f(n, 2);
        Override o(1);
        Earlier e(1);
        if (d.value != 4 || d.extra != 5 || f.value != 5 || f.extra != 5 || o.value != 11 || e.value != 21) return 1;
    }
    return destroyed == 41 ? 0 : 2;
}
