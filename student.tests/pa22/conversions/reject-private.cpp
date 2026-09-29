struct A { operator int() const { return 1; } };
struct Pad {};
struct D : Pad, private A {};
int main() { D d; return int(d); }
