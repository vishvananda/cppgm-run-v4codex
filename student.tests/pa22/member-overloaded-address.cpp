struct S {int x;int f()const{return x+1;}};
typedef int(S::*Pointer)()const;
int calls;
struct Proxy {Pointer operator&()const {++calls;return &S::f;}};
int main(){S s;s.x=7;Proxy proxy;Pointer p=&proxy;return calls!=1 || (s.*p)()!=8;}
