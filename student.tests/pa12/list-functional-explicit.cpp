struct X { int n; explicit X(int x):n(x){} };
int main() { X x = X{7}; return x.n != 7; }
