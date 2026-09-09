struct X { explicit operator bool() const { return true; } };
int main() { X x; bool value = x; return value; }
