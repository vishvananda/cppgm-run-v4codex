struct Value {int x; int add(int y) const {return x+y;} };
int reads;
struct Pointer {Value* p; Value& operator*() const {++reads; return *p;} };
template<int N> int fixed(Pointer p,int v) {return __builtin_invoke(&Value::add,p,v)+N;}
template<int N> int& data(Pointer p) {return __builtin_invoke(&Value::x,p);}
template<class T> auto dependent(T& p)->decltype(__builtin_invoke(&Value::x,p)){return __builtin_invoke(&Value::x,p);}
int main(){ Value v={4}; Pointer p={&v}; if(fixed<1>(p,2)!=7 || fixed<2>(p,3)!=9 || reads!=2)return 1;
 data<1>(p)=8; data<2>(p)=9; dependent(p)=10; return v.x==10 && reads==5?0:2; }
