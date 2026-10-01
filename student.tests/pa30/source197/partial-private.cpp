template<class...> using discard = void;
template<class...> struct pack {};
template<class A,class B,class C=void> struct choice;
template<class A,class... B> struct choice<A,pack<B...>,discard<typename A::type>> { static const int value=1; };
template<class A,class B> struct choice<A,B,void> { static const int value=2; };
class hidden { typedef int type; };
static_assert(choice<hidden,pack<int>>::value==2,"erased alias retains private access obligation");
int main() { return 0; }
