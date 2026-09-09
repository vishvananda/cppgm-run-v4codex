int n; struct A{int x; A():x(++n){}};
A a[3];
namespace space { A b[12]; }
thread_local A c[2];
int main(){if(n!=15 || a[2].x!=3 || space::b[11].x!=15)return 1; if(c[0].x!=16 || c[1].x!=17)return 2;return n!=17;}
