template<class T>struct Count{operator int()const{typename T::missing bad;return 3;}};
using P=decltype(new int[Count<int>()]);
static_assert(__is_same(P,int*),"unevaluated conversion does not demand a body");
struct Signed{operator int()const;};
template<class T>auto form(T n)->decltype(new int[n]);
static_assert(__is_same(decltype(form(Signed())),int*),"substituted bound");
int main(){return 0;}
