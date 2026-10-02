int main() { int x=2; struct A { int get() { decltype(x) y=3; return sizeof(x)+sizeof(&x)+y; } }; return A().get()-15; }
