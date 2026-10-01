struct Empty{};struct Base{int a,b;};struct Derived:Empty,Base{Derived(){a=3;b=5;}};
struct Virtual:virtual Base{Virtual(){a=4;b=7;}};
int main(){Derived d;auto& [x,y]=d;x=9;Virtual v;auto& [a,b]=v;b=8;return d.a!=9||y!=5||a!=4||v.b!=8;}
