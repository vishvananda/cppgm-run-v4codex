// Combined ownership trace: nested demand, lists, inherited defaults and emission.
template<class T> struct Outer {
  struct Dormant { typename T::missing member; };
  struct Base { int n; template<class U> constexpr Base(U n, int k=2):n(n+k){} };
  struct Derived:Base { using Base::Base; };
};
template<class T> struct Value { static const int n=T{3}.n; };
static_assert(Value<Outer<int>::Derived>::n==5, "constant first");
template<class T> auto test(int)->decltype(new T{1.5},char());
template<class> long test(...);
static_assert(sizeof(test<int>(0))==sizeof(long), "narrowing is immediate");
struct Dead { Dead(int); ~Dead()=delete; };
template<class T> auto allocated(int)->decltype(new T{1},char());
template<class> long allocated(...);
static_assert(sizeof(allocated<Dead>(0))==1, "allocated object is not a temporary");
void declaration(Outer<int>::Dormant);
int effects;
int seed(){++effects;return 7;}
int main(){Outer<int>::Derived d{seed()};return d.n!=9||effects!=1;}
