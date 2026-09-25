struct X { operator double() const { return 0.5; } };
int main() { int x{X()}; return x; }
