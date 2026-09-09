int trace;
struct Number {
    int value;
    Number(int n) : value(n) {}
    int& operator[](int) { return value; }
    int operator()() const { return value; }
    Number& operator++() { ++value; return *this; }
    int operator++(int) { int old = value; ++value; return old; }
    friend int operator+(const Number& a, const Number& b) { return a.value + b.value; }
    friend bool operator&&(const Number&, const Number&) { ++trace; return false; }
    friend int operator,(const Number& a, const Number& b) { ++trace; return a.value + b.value; }
};
struct Root {};
struct Base : Root {};
struct Derived : Base {};
int select(Root&) { return 1; }
int select(Base&) { return 2; }
int main() {
    Number a(3), b(4);
    if (a && b) return 1;
    (a,b);
    if (trace != 2 || a + b != 7) return 2;
    a[0] = 8;
    if ((a++) != 8 || (++a)() != 10 || Number(11)() != 11) return 3;
    Derived d;
    return select(d) != 2;
}
