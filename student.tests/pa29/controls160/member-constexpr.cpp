struct Base {int x; constexpr int add(int y) const {return x+y;} };
struct Pointer {const Base* p; constexpr const Base& operator*() const {return *p;} };
constexpr Base b={9};
constexpr Pointer p={&b};
constexpr int a=__builtin_invoke(&Base::x,b);
constexpr int c=__builtin_invoke(&Base::x,&b);
constexpr int d=__builtin_invoke(&Base::x,p);
constexpr int e=__builtin_invoke(&Base::add,b,2);
constexpr int f=__builtin_invoke(&Base::add,&b,3);
constexpr int g=__builtin_invoke(&Base::add,p,4);
static_assert(a==9 && c==9 && d==9 && e==11 && f==12 && g==13,"constexpr invoke");
template<int N> struct Result {static const int value=__builtin_invoke(&Base::add,p,N);};
static_assert(Result<7>::value==16,"dependent constant query");
int main(){return 0;}
