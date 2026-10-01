struct Value{int x; int add(int y) const {return x+y;}};
template<class T> struct Ptr{T* p;};
template<class T> T& operator*(const Ptr<T>& p) {return *p.p;}
int main(){Value v={7};Ptr<Value> p={&v};
 if(__builtin_invoke(&Value::add,v,2)!=9)return 1;
 return __builtin_invoke(&Value::add,p,4)==11?0:2;}
