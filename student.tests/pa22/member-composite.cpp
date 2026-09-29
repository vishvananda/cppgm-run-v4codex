struct A {int a;}; struct B {int b;int f()const{return b;}}; struct D:A,B{};
int main() {
 D d;d.a=15;d.b=8; int B::*bp=&B::b; const int D::*dp=bp;
 if(bp!=dp) return 1;
 bool choose=false; const int D::*p=choose ? bp : dp;
 if(d.*p!=8) return 2;
 int(B::*bf)()const=&B::f; int(D::*df)()const=bf;
 if(bf!=df) return 3;
 auto fn=choose ? bf : df;
 return (d.*fn)()!=8;
}
