typedef int (^Callback)(int);
typedef int (*Function)(int);
template<class T> struct Shape { typedef T (^Block)(T); };
template<class T> struct Identity { static const int value = 0; };
template<class R,class A> struct Identity<R (^)(A)> { static const int value = 1; };
static_assert(sizeof(Callback)==8 && alignof(Callback)==8,"block reference storage");
static_assert(__is_same(Shape<int>::Block,Callback),"substitution");
static_assert(!__is_same(Callback,Function),"distinct type");
static_assert(!__is_pointer(Callback) && __is_scalar(Callback),"block shape");
static_assert(__is_trivial(Callback) && __is_standard_layout(Callback),"scalar storage");
static_assert(Identity<Callback>::value==1 && Identity<Function>::value==0,"partial deduction");
constexpr Callback empty = nullptr;
static_assert(!empty && empty==nullptr && empty==0,"null constants");
struct Holder { Callback value; int n; };
int overload(Callback){return 1;}
int overload(Function){return 2;}
int target(int x){return x;}
int main() {
 Callback value = nullptr;
 Callback const& alias = value;
 Callback* address = &value;
 Callback array[3] = {};
 Holder a = {value,3}; Holder b = a;
 void* opaque = value;
 Callback recovered = reinterpret_cast<Callback>(opaque);
 return overload(value)!=1 || overload(&target)!=2 || alias || *address || array[2] || b.value ||
  recovered || (true ? value : nullptr) || (false ? nullptr : value);
}
