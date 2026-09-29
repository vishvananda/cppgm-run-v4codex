struct A { operator int() const { return 1; } };
struct B : A {};
struct C : A {};
struct D : B, C {};
int main() { D d; return int(d); }
