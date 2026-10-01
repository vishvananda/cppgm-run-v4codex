struct Value{int x; int add(int a) const{return x+a;} };
namespace ptr {
struct Handle {Value* p;};
Value& operator*(Handle h) noexcept {return *h.p;}
template<class T> struct Generic {T* p;};
template<class T> T& operator*(Generic<T> const& h) noexcept {return *h.p;}
}
template<class F, class... Args> auto invoke(F f,Args&&... args)->decltype(__builtin_invoke(f,args...)) {return __builtin_invoke(f,args...);}
int main(){Value v={8}; ptr::Handle h={&v}; ptr::Generic<Value> g={&v};
 if(invoke(&Value::add,h,4)!=12)return 1;
 invoke(&Value::x,g)=9;
 return __builtin_invoke(&Value::add,g,4)==13?0:2;}
