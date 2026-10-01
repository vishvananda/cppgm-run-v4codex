int alive,reads;
struct Value {int x; Value(int v):x(v){++alive;} Value(const Value& v):x(v.x){++alive;} ~Value(){--alive;} int read() const {return x+alive;} };
struct Pointer {Value operator*() const {++reads; return Value(7);} };
struct Throw { Value& operator*() const {throw 3;} };
int main(){
 int a=__builtin_invoke(&Value::x,Pointer{});
 if(a!=7 || reads!=1 || alive) return 1;
 int b=__builtin_invoke(&Value::read,Pointer{});
 if(b!=8 || reads!=2 || alive) return 2;
 try {Value guard(4); (void)__builtin_invoke(&Value::x,Throw{}); return 3;} catch(int n) {if(n!=3 || alive) return 4;}
 return 0;
}
