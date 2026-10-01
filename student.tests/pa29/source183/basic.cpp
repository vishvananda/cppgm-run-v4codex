template<class T> struct box { int value; };
box(const char*) -> box<int>;
box(int) -> box<long>;
int main() { box<int> b{7}; return b.value != 7; }
