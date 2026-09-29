struct A { operator int() const { return 1; } };
struct B { operator int() const { return 2; } };
struct D : A, B {};
int main() { D d; return int(d); }
