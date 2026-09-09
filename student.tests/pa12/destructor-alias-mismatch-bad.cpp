struct A { ~A() {} };
struct B {};
int main() { A a; using Wrong=B; a.~Wrong(); }
