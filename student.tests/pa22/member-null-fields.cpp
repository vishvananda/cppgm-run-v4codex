struct S { int m; int f() {return m;} };
struct Holder { int S::* p; int (S::* f)(); };
Holder global = {};
int main() {
 Holder h = {}; Holder a[10] = {};
 if (h.p || h.f || global.p || global.f || a[9].p || a[9].f) return 1;
 S s; s.m=19;
 h.p=&S::m; h.f=&S::f;
 Holder copy=h;
 return s.*copy.p!=19 || (s.*copy.f)()!=19;
}
