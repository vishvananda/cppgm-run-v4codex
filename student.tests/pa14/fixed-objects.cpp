// Fixed object/member facts retain symbolic receivers across specializations.
int live=0,reads=0;
struct Value {
    int n;
    Value(int n):n(n){++live;}
    Value(const Value& v):n(v.n){++live;}
    ~Value(){--live;}
    int get() const {return n;}
};
struct Base {
    int n;
    Base(int n):n(n){}
    virtual int get(int extra) const {return n+extra;}
    int base() const {return n;}
};
struct Object:Base {
    mutable int calls;
    int& referred;
    Object(int n,int& r):Base(n),calls(0),referred(r){}
    int get(int extra) const override {++calls;return n+extra+10;}
    int add(int extra) & {n+=extra;return n;}
    int add(int extra) && {return n+extra+20;}
    Value value() const {return Value(n);}
    static int fixed(int n){return n+3;}
};
Object* current;
Object& read(){++reads;return *current;}
Value produce(int n){return Value(n);}
int inspect(Value value){return value.n;}
template<class T>int fields(Object& object,int delta){object.n+=delta;object.referred+=delta;return object.n+object.referred;}
template<class T>int member(Object& object,int n){return object.add(n)+object.base();}
template<class T>int virtual_call(const Base& object,int n){return object.get(n);}
template<class T>int qualified(const Object* object,int n){return object->Base::get(n);}
template<class T>int mutable_field(const Object& object){return ++object.calls;}
template<class T>int local(int n){int r=0;Object object(n,r);return object.add(2)+object.n;}
template<class T>int results(const Object& object){return inspect(object.value());}
template<class T>int temporary(int n){return produce(n).get();}
template<class T>int static_receiver(int n){return read().fixed(n);}
template<class T>int parenthesized(Object& object){return (object.base)();}
template<class T>int rvalue_call(Object& object,int n){return static_cast<Object&&>(object).add(n);}
template<class T>int scalar_pointer(int* p){return *p;}
int twice(int n){return n*2;}
struct Callback {int (*fn)(int);};
template<class T>int callback(const Callback& c,int n){return c.fn(n);}
struct Functor {int operator()(int n)const{return n+6;}};
template<class T>int callable(const Functor& f,int n){return f(n);}
int referred_value=0;
struct References {int& member;static int data;};
int References::data=0;
References references(){References r={referred_value};return r;}
template<class T>int& reference_result(){return references().member;}
template<class T>int& static_result(){return references().data;}
template<class T>auto queried_value()->decltype((produce(1).n));
char category(int(*)());
long category(int&&(*)());
template<class T>auto queried_reference()->decltype((references().member));
char reference_category(int&&(*)());
long reference_category(int&(*)());
struct Enumerated {enum Code {one=1};};
Enumerated enumerated();
template<class T>auto queried_enumerator()->decltype((enumerated().one));
char enum_category(Enumerated::Code&&(*)());
long enum_category(Enumerated::Code(*)());
struct Tag{};
int main(){
    int x=4,y=5;Object a(2,x),b(3,y);
    if(fields<int>(a,1)!=8 || fields<Tag>(b,2)!=12 || x!=5 || y!=7) return 1;
    if(member<int>(a,2)!=10 || member<Tag>(b,3)!=16) return 2;
    if(virtual_call<int>(a,1)!=16 || virtual_call<Tag>(b,2)!=20 || a.calls!=1 || b.calls!=1) return 3;
    if(qualified<int>(&a,1)!=6 || qualified<Tag>(&b,2)!=10 || a.calls!=1 || b.calls!=1) return 4;
    if(mutable_field<int>(a)!=2 || mutable_field<Tag>(b)!=2) return 5;
    if(local<int>(3)!=10 || local<Tag>(4)!=12) return 6;
    if(results<int>(a)!=5 || live || results<Tag>(b)!=8 || live) return 7;
    if(temporary<int>(6)!=6 || live || temporary<Tag>(7)!=7 || live) return 8;
    current=&a;
    if(static_receiver<int>(2)!=5 || static_receiver<Tag>(3)!=6 || reads!=2) return 9;
    if(parenthesized<int>(a)!=5 || parenthesized<Tag>(b)!=8) return 10;
    if(scalar_pointer<int>(&x)!=5 || scalar_pointer<Tag>(&y)!=7) return 11;
    reference_result<int>()=21;static_result<int>()=22;
    if(reference_result<Tag>()!=21 || static_result<Tag>()!=22) return 12;
    if(sizeof(category(queried_value<int>))!=sizeof(long)) return 13;
    if(sizeof(reference_category(queried_reference<int>))!=sizeof(long)) return 14;
    if(sizeof(enum_category(queried_enumerator<int>))!=sizeof(long)) return 15;
    if(rvalue_call<int>(a,2)!=27 || rvalue_call<Tag>(b,3)!=31) return 16;
    Callback cb={twice};
    if(callback<int>(cb,3)!=6 || callback<Tag>(cb,4)!=8) return 17;
    Functor functor;
    if(callable<int>(functor,2)!=8 || callable<Tag>(functor,3)!=9) return 18;
    return 0;
}
