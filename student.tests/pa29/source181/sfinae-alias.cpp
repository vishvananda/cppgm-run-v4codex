template<int N> using A=int[N];
template<int N> char f(A<N>*);
template<int N> long f(...);
static_assert(sizeof(f<0>((A<0>*)0))==sizeof(long),"alias zero substitution");
template<int N> struct Holder{typedef int type[N];};
template<int N> char g(typename Holder<N>::type*);
template<int N> long g(...);
static_assert(sizeof(g<0>((int(*)[0])0))==sizeof(char),"class definition is not immediate");
int main(){return 0;}
