// Even an empty user constructor must initialize every element's vpointer.
struct B { B() {} virtual int f() const { return 7; } virtual ~B() {} };
int main() {
    volatile int n = 5;
    B* objects = new B[n];
    int sum = 0;
    for (int i = 0; i < n; ++i) sum += objects[i].f();
    delete[] objects;
    return sum != 35;
}
