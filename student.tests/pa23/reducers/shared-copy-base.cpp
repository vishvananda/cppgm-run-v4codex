struct V { int value; V(int v = 3) : value(v) {} };
struct A : virtual V {};
struct D : A { D(const A& a) : V(9), A(a) {} };
int main() { A a; a.value = 7; D d(a); return d.value != 9; }
