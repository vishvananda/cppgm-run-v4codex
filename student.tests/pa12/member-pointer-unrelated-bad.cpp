struct A { int n; }; struct B { int n; };
int main() { A a={}; int B::*field=&B::n; return a.*field; }
