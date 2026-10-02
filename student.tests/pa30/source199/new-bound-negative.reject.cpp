template<class T> void f() { new T[-1]; }
int main() { f<int>(); }
