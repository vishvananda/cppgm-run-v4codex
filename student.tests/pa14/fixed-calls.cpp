// Fixed selection is shared; references, converted arguments and result objects
// belong to the evaluated occurrence. C++11 [temp.res], [expr.call], [class.temporary].
int longs=0,ints=0,defaults=0,live=0;
int choose(long n){++longs;return n+10;}
template<class T>int fixed(int n){return choose(n);}
int choose(int n){++ints;return n+100;}
int default_value(){++defaults;return 7;}
int with_default(int n,int extra=default_value()){return n+extra;}
template<class T>int default_call(int n){return with_default(n);}
int twice(int n){return n*2;}
template<class T>int indirect(int (*fn)(int),int n){return (fn)(n);}
template<class T>int by_reference(int (&fn)(int),int n){return fn(n);}
int& bump(int& n){++n;return n;}
template<class T>int& reference(int& n){return bump(n);}
template<class U>U target(U n){return n+n;}
template<class T>int explicit_call(int n){return target<int>(n);}
template<class U>int bad_body(){return U::missing;}
template<class T>int unused(){return bad_body<int>();}
struct Value {
    int n;
    Value(int v):n(v){++live;}
    Value(const Value& v):n(v.n){++live;}
    ~Value(){--live;}
    operator int() const {return n;}
};
int inspect(Value v){return v.n;}
Value produce(int n){return Value(n);}
int scalar(int n){return n;}
template<class T>int constructed(int n){return inspect(n);}
template<class T>int result(int n){return inspect(produce(n));}
template<class T>int converted(int n){return scalar(produce(n));}
int default_object(Value v=produce(5)){return v.n;}
template<class T>int object_defaults(){return default_object()+default_object();}
struct Static { static int run(int n){return n+3;} };
template<class T>int static_call(int n){return Static::run(n);}
struct Tag{};
namespace adl {
struct Key {
    int n;
    Key(int v):n(v){}
    operator int() const {return n;}
    friend int pick(const Key& v){return v.n+2;}
};
Key make(int n){return Key(n);}
}
int pick(int n){return n+20;}
template<class T>int associated(int n){return pick(adl::make(n));}
struct Protected { protected: static int inherited(int n){return n+4;} };
template<class T>struct Derived:Protected {int run(int n){return inherited(n);}};
template<class T>int local_access(int n){struct Local:Protected{static int run(int n){return inherited(n);}};return Local::run(n);}
int main(){
    if(fixed<int>(3)!=13 || fixed<Tag>(4)!=14 || longs!=2 || ints) return 1;
    if(default_call<int>(2)!=9 || default_call<Tag>(3)!=10 || defaults!=2) return 2;
    if(indirect<int>(twice,4)!=8 || indirect<Tag>(twice,6)!=12) return 3;
    int a=1,b=3;reference<int>(a)=7;reference<Tag>(b)=9;
    if(a!=7 || b!=9) return 4;
    if(explicit_call<int>(3)!=6 || explicit_call<Tag>(4)!=8) return 5;
    if(constructed<int>(8)!=8 || live || constructed<Tag>(9)!=9 || live) return 6;
    if(result<int>(11)!=11 || live || result<Tag>(12)!=12 || live) return 7;
    if(converted<int>(13)!=13 || live || converted<Tag>(14)!=14 || live) return 8;
    if(static_call<int>(2)!=5 || static_call<Tag>(3)!=6) return 9;
    if(sizeof(with_default(0))!=sizeof(int) || defaults!=2) return 10;
    if(associated<int>(4)!=6 || associated<Tag>(5)!=7) return 11;
    if(by_reference<int>(twice,4)!=8 || by_reference<Tag>(twice,5)!=10) return 12;
    if(object_defaults<int>()!=10 || live || object_defaults<Tag>()!=10 || live) return 13;
    Derived<int> d;Derived<Tag> e;
    if(d.run(2)!=6 || e.run(3)!=7 || local_access<int>(4)!=8 || local_access<Tag>(5)!=9) return 14;
    return 0;
}
