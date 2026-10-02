template<class T> int f(T x) { struct A { int get() { return x; } }; return A().get(); } int main(){return f(1);}
