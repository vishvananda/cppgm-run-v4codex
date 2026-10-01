int other(int) { return 9; }
struct F {
    static int operator()(int x) { return x+1; }
    using P=int(*)(int);
    operator P() const { return other; }
};
int main() { return F()(4)!=5; }
