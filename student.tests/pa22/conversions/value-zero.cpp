typedef unsigned long size_t;
void* operator new(size_t, void* p) noexcept { return p; }
struct A { int a; };
struct B { int b; };
struct C : A, B { int c; C() = default; };
struct User : A, B { User() { a = 11; b = 12; } };
struct WithBaseConstructor : User { int tail; };
struct Late : A { Late(); };
Late::Late() = default;
struct Outer { C c; C array[12]; User user; WithBaseConstructor nested; Late late; };
int main() {
    alignas(Outer) unsigned char bytes[sizeof(Outer)];
    for (size_t i = 0; i < sizeof(bytes); ++i) bytes[i] = 1;
    Outer* p = new (bytes) Outer{};
    if (p->c.a || p->c.b || p->c.c) return 1;
    for (int i = 0; i < 12; ++i)
        if (p->array[i].a || p->array[i].b || p->array[i].c) return 2;
    if (p->user.a != 11 || p->user.b != 12) return 3;
    if (p->nested.a != 11 || p->nested.b != 12 || p->nested.tail) return 5;
    // An out-of-class defaulted constructor is user-provided, so value
    // initialization leaves its int's existing object representation alone.
    unsigned char* late = reinterpret_cast<unsigned char*>(&p->late);
    for (size_t i = 0; i < sizeof(Late); ++i) if (late[i] != 1) return 4;
    return 0;
}
