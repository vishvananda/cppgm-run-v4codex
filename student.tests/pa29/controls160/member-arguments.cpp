int live,copies,conversions,pointers;
struct Arg {int x; Arg(int a):x(a){++live;} Arg(const Arg& a):x(a.x){++live;++copies;} ~Arg(){--live;} };
struct Int {operator int() const {++conversions;return 4;}};
struct Value {int x; int add(int y) const {return x+y;} int take(Arg a){return x+a.x+live;} int& ref(){return x;} void set(int y){x=y;} };
typedef int(Value::*PM)(int) const;
PM pointer(){++pointers;return &Value::add;}
int main(){Value v={7};
 if(__builtin_invoke(pointer(),v,Int{})!=11 || pointers!=1 || conversions!=1)return 1;
 {Arg a(3); if(__builtin_invoke(&Value::take,&v,a)!=12 || live!=1 || copies!=1)return 2;}
 if(live)return 3;
 __builtin_invoke(&Value::ref,&v)=9;
 __builtin_invoke(&Value::set,v,12);
 return v.x==12?0:4;}
