struct Base{int a;};struct L:Base{};struct R:Base{};struct Derived:L,R{};int main(){Derived d;auto& [x]=d;}
