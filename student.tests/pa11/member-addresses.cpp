struct Base {
    int value;
    int &ref() { return this->value; }
    int get() const { return value; }
    int pick(int) { return 5; }
    int pick(int) const { return 7; }
    static int twice(int n) { return n * 2; }
};
struct Derived : Base { long tail; };
int main() {
    Derived d;
    d.value = 13;
    d.tail = 31;
    Base &b = d;
    b.ref() += 4;
    const Base &c = d;
    return c.get() == 17 && b.pick(0) == 5 && c.pick(0) == 7 &&
           d.tail == 31 && d.twice(9) == 18 && sizeof(Derived) == 16 ? 0 : 1;
}
