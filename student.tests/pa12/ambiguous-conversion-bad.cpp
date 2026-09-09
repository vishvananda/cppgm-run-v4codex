struct X { operator int() const { return 1; } operator double() const { return 2; } };
int main() { X x; long value = x; return value; }
