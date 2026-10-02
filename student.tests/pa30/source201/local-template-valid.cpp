template<class T> int f(T x) {
 const int k=3; struct A { int get() { return k+sizeof(x); } };
 return A().get()+[&] { return [=] { return x; }(); }();
} int main(){return f(5)-12;}
