template<int N> struct S { int tag; int array[N]; };
template<class T> struct Shape { static constexpr int value = 1; };
template<class T> struct Shape<T[]> { static constexpr int value = 2; };
template<class T, unsigned long N> struct Shape<T[N]> { static constexpr int value = 3; };
static_assert(sizeof(S<0>) == sizeof(int), "substituted zero");
static_assert(Shape<int[]>::value == 2, "unknown partial specialization");
static_assert(Shape<int[0]>::value == 1, "known partial specialization");
template<class T> using Const = const T;
static_assert(__is_same(Const<int[0]>, const int[0]), "alias shape");
int main() { S<0> x{9,{}}; S<0> y = x; return y.tag != 9; }
