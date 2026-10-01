struct A {int a; A():a(13){}};
struct Pad {int p; Pad():p(3){}};
struct D:Pad,private A {int value()const{return a;}};
int main(){D d; const D* cd=&d; A *a=(A*)cd; a->a=17;
 D *p=(D*)(const A*)a;
 A& ar=(A&)(const D&)d; ar.a=19;
 int A::*m=&A::a; int D::*dm=(int D::*)m;
 return p==&d && d.value()==19 && d.*dm==19?0:1;}
