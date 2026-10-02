void f() { const int x=1; struct A { int get() { return [x] { return x; }(); } }; }
