template<class T> char test(int T::*);
template<class T> long test(...);
struct A {}; union U { int x; }; enum E{e};
static_assert(sizeof(test<int>(0))==sizeof(long),"nonclass");
static_assert(sizeof(test<E>(0))==sizeof(long),"enum");
static_assert(sizeof(test<A>(0))==sizeof(char),"class");
static_assert(sizeof(test<U>(0))==sizeof(char),"union");
template<class T> int check(T A::*);
template<class T> char check(...);
static_assert(sizeof(check<void>(0))==sizeof(char),"void member");
static_assert(sizeof(check<int&>(0))==sizeof(char),"reference member");
int main(){return 0;}
