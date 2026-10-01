template<class T> struct Lazy { inline static int invalid = T::missing; };
int main() { return Lazy<int>::invalid; }
