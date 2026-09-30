struct B { int prefix; struct { int a; }; B(int x):prefix(3),a(x){} };
struct L:B{L():B(7){}}; struct R:B{R():B(9){}};
struct D:L,R{int read(){return L::a+R::a;}};
int main(){ D d; return d.L::a==7 && d.R::a==9 && d.read()==16 ? 0:1; }
