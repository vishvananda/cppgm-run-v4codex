template<class T>int f(T x){struct A{int g(int y=x){return y;}};return A().g();}int main(){return f(1);}
