struct A { int x; };
struct B : A {};
struct Pad { int pad; };
struct C : Pad, B {};
char (&select(A))[1];
char (&select(B))[2];
int pointer(A*) { return 1; }
int pointer(B*) { return 2; }
int reference(const A&) { return 1; }
int reference(const B&) { return 2; }
int value(A a) { return a.x + 1; }
int value(B b) { return b.x + 2; }
template<class T> int size() { return sizeof(select(T())); }
int main() {
    C c; c.x = 7;
    return size<C>() != 2 || pointer(&c) != 2 || reference(c) != 2 || value(c) != 9;
}
