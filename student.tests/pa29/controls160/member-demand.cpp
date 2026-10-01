template<class T>T&& declval() noexcept;
struct Base {int x; int call(int);};
template<class T> struct Ptr {Base& operator*() const {return T::missing;} };
static_assert(__is_same(decltype(__builtin_invoke(&Base::x,declval<Ptr<int>>())),int&),"do not demand dereference body");
static_assert(__is_same(decltype(__builtin_invoke(&Base::call,declval<Ptr<int>>(),2)),int),"do not demand callable body");
int main(){return 0;}
