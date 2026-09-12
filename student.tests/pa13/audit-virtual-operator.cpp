// PA12's selected member operators participate in PA13 virtual dispatch.
int selected;
struct B {
    virtual B& operator=(int n) { selected = n; return *this; }
    virtual int operator()(int n) const { return n; }
};
struct D : B {
    D& operator=(int n) override { selected = n + 10; return *this; }
    int operator()(int n) const override { return n + 20; }
};
int main() {
    D d;
    B& b = d;
    b = 7;
    if (selected != 17 || b(3) != 23) return 1;
    b.B::operator=(2);
    return selected != 2;
}
