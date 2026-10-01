template<int N> struct A { static int a[N]; };
template<int N> int A<N>::a[N] = {};
template<int N> struct B { static int a[]; };
template<int N> int B<N>::a[N] = {};
static_assert(sizeof(A<0>::a)==0, "definition zero");
static_assert(sizeof(B<0>::a)==0, "definition fills absent bound");
int main(){return A<0>::a != A<0>::a || B<0>::a != B<0>::a;}
