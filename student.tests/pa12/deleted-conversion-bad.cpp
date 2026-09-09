struct X { operator int() & { return 1; } operator int() && = delete; };
int main() { return X(); }
