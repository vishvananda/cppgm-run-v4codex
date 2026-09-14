// N3485: 3.6.2 [basic.start.init]/2; 5.19 [expr.const].
int read();int seen=read();struct E{const char*p;constexpr E(const char*p):p(p){}};template<class T>struct S{static constexpr E e=E("one");};template<class T>constexpr E S<T>::e;int read(){return S<int>::e.p[0];}int main(){return seen==111?0:1;}
