template<class T> struct LazyArray { inline static int array[] = {T::missing}; };
int main() { return sizeof(LazyArray<int>::array); }
