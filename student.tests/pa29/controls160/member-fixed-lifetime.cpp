int alive;
struct Value {int x;Value():x(7){++alive;} Value(const Value& p):x(p.x){++alive;} ~Value(){--alive;} };
struct Ptr{Value operator*() const {return Value();}};
template<int N> int get(Ptr p){return __builtin_invoke(&Value::x,p)+N;}
int main(){if(get<1>(Ptr{})!=8 || alive)return 1;return get<2>(Ptr{})==9 && !alive?0:2;}
