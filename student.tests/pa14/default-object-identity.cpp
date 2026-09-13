int live=0,counter=0,destroyed=0;
struct Value {
    int n;
    Value(int n):n(n){++live;}
    Value(const Value& v):n(v.n){++live;}
    ~Value(){destroyed+=n;--live;}
};
Value make(){return Value(++counter);}
const Value& value(const Value& v=make()){return v;}
int distinct(const Value& a,const Value& b){return a.n!=b.n && a.n+b.n==2*counter-1 && live==2;}
template<class T>int check(){return distinct(value(),value());}
template<class T>const T& generic(const T& v=make()){return v;}
int ordinary(){return distinct(value(),value());}
int dependent(){return distinct(generic<Value>(),generic<Value>());}
int conditional(bool yes){return yes ? distinct(value(),value()) : distinct(value(),value());}
struct Convert { operator Value() const {return make();} };
const Value& converted(const Value& v=Convert()){return v;}
int conversion(){return distinct(converted(),converted());}
struct Argument {
    int n;
    Argument(int, const Value& v=make()):n(v.n){}
};
int arguments(Argument a,Argument b){return a.n!=b.n && a.n+b.n==2*counter-1 && live==2;}
template<class T>int constructors(){return arguments(1,2);}
struct Plain {int n;};
Plain make_plain(){Plain p={++counter};return p;}
const Plain& plain(const Plain& v=make_plain()){return v;}
bool plain_distinct(const Plain& a,const Plain& b){return &a!=&b && a.n!=b.n && a.n+b.n==2*counter-1;}
int array_bad=0;
struct Element {Element(const Value& v=make()){if(live!=1 || v.n!=counter)array_bad=1;}};
template<class T>void arrays(){Element small[3];Element large[12];}
struct Tag{};
int main(){
    if(!check<int>() || live || destroyed!=3 || !check<Tag>() || live || destroyed!=10) return 1;
    if(!ordinary() || live || destroyed!=21 || !dependent() || live || destroyed!=36) return 2;
    if(!conditional(true) || live || destroyed!=55 || !conditional(false) || live || destroyed!=78) return 3;
    if(!conversion() || live || destroyed!=105) return 4;
    if(!constructors<int>() || live || destroyed!=136 || !constructors<Tag>() || live || destroyed!=171) return 5;
    if(!plain_distinct(plain(),plain())) return 6;
    counter=destroyed=0;arrays<Tag>();
    if(array_bad || live || counter!=15 || destroyed!=120)return 7;
    return 0;
}
