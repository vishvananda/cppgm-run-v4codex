struct B { int n; B(int x):n(x){} virtual int f()const noexcept{return n;} };
struct D:B { D(int x):B(x){} int f()const noexcept override{return n+100;} };
int main(){D d(3);B b(9);B& target=d;target=b;return target.f()!=109;}
