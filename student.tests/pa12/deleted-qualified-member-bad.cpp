struct X { int pick() & { return 1; } int pick() && = delete; };
int main() { X x; return static_cast<X&&>(x).pick(); }
