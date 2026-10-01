struct Base{int a,b;};struct Derived:Base{constexpr Derived():Base{3,4}{}};
constexpr int f(){Derived d;auto& [x,y]=d;x=8;return x+y;}
static_assert(f()==12,"base projection");int main(){return f()!=12;}
