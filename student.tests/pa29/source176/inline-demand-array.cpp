template<class T> struct LazyArray {
 inline static int array[] = {T::missing};
 inline static int selected[] = {2,3};
 inline static constexpr int constants[] = {4,5,6};
};
static_assert(sizeof(LazyArray<int>::constants)==3*sizeof(int), "constexpr bound demand");
int main() { LazyArray<int> a; return sizeof(a) + sizeof(LazyArray<int>::selected)/sizeof(int) - 3; }
