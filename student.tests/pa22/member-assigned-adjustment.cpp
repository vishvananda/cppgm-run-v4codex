struct A {int a;};struct B {int b;int f()const{return b;}};
struct D:A,B {int d;int g()const{return d;}};
int main(){D d;d.a=1;d.b=2;d.d=3;int(B::*p)()const=&B::f;
 if(d.a) p=static_cast<int(B::*)()const>(&D::g);
 return (d.*p)()!=3;}
