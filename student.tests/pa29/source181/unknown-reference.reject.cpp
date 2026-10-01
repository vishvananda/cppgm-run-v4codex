void f(int (&)[0]); extern int a[]; void g() { f(a); }
