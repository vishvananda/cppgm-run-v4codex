struct fixed { fixed() = delete; int get(); };
template<class T> struct unused { int get() { return fixed().get(); } };
int main() { return 0; }
