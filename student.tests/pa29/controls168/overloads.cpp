struct A {int a;int b;};struct B{int x;};struct C{C(int);};
int choose(int){return 1;}int choose(B){return 2;}int choose(C){return 3;}int choose(A a){return a.a==0 && a.b==7?4:5;}
template<class T> auto viable(int)->decltype((T){.b=2},int()){return 1;}
template<class> int viable(...){return 2;}
int main(){return choose({.b=7})!=4 || viable<int>(0)!=2 || viable<C>(0)!=2 || viable<A>(0)!=1;}
