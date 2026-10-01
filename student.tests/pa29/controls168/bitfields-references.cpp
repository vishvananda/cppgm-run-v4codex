struct S {unsigned a:3;unsigned b:5;int& ref;};
int main(){int x=7;S s={.b=19,.ref=x};s.ref=9;return s.a!=0 || s.b!=19 || x!=9;}
