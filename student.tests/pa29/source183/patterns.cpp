template<class T, int N = 1> struct box { T value; };
template<class U> box(U) -> box<U>;
template<class U> box(U*) -> box<U*, 2>;
template<class U, int N> box(U (&)[N]) -> box<U, N>;
template<class U> box(U(*)(int)) -> box<U>;
int main(){box<int> b{9};return b.value!=9;}
