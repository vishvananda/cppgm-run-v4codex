template<class R,class A> using Block = R (^)(A);
typedef int Function(int);
typedef Function^ Alias;
static_assert(__is_same(Block<int,int>,Alias),"function alias block");
template<class A> auto probe(int)->decltype(sizeof(Block<int,A>));
template<class> char probe(...);
static_assert(sizeof(probe<int>(0))==8,"valid alias");
static_assert(sizeof(probe<void>(0))==1,"void parameter substitution failure");
struct Header {void* isa;int flags,reserved;int (*invoke)(void*,int);void* descriptor;int capture;};
int entry(void* raw,int value){return static_cast<Header*>(raw)->capture+value;}
typedef int (^Callback)(int);
struct Callable {
 Callback value;
 operator Callback() const { return value; }
};
template<class T> int fixed(Callback value) {return value(1);}
template<class T> auto queried(T value)->decltype(__builtin_invoke(value,2)) {return __builtin_invoke(value,2);}
template<class T> constexpr bool can_throw(T value){return noexcept(value(1));}
static_assert(!can_throw(Callback()),"indirect call may throw");
int main() {
 Header object={0,0,0,entry,0,5}; Callback callback=(int (^)(int))&object;
 Callable wrapper={callback};
 if(fixed<long>(callback)!=6 || queried(callback)!=7)return 1;
 if(static_cast<Callback>(wrapper)(3)!=8 || !wrapper)return 2;
 Callback empty=nullptr;
 if((true ? wrapper : empty)(4)!=9)return 3;
 return 0;
}
