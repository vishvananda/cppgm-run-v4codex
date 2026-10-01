template<class T> struct Self { inline static int value = value + 1; };
int main() { return Self<int>::value-1; }
