template<class T> void f(unsigned n) { new T[2][n]; }
int main() { f<int>(3); }
