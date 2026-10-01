struct Base{int a;};struct Derived:Base{int b;};int main(){Derived d;auto& [x,y]=d;}
