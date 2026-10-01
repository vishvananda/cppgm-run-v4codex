constexpr int number = 17;
template<class T> constexpr T* unqualify(const T* p) {return (T*)p;}
template<class T> constexpr T& unqualify_ref(const T& p) {return (T&)p;}
template<class T> constexpr T read(const T* p) {return *unqualify(p);}
template<class T, int N = read(&number)> struct Value {static const int value = N;};
static_assert(read(&number) == 17, "substitution");
static_assert(unqualify_ref(number) == 17, "reference substitution");
static_assert(Value<int>::value == 17, "default argument");
template<class T, class U> auto viable(T p, int) -> decltype(const_cast<U>(p), char());
template<class T, class U> long viable(T p, ...);
static_assert(sizeof(viable<const int*, int*>(nullptr, 0)) == 1, "viable");
static_assert(sizeof(viable<const int*, float*>(nullptr, 0)) == sizeof(long), "substitution failure");
int main(int argc, char**) {int n=argc; *unqualify(&n)=7; return n==7?0:1;}
