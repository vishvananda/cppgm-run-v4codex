struct A { operator int() const { return 1; } };
struct B : A { operator int() const { return 2; } };
struct C : A {};
struct D : B, C {};
int main() { D d; return int(d); }
