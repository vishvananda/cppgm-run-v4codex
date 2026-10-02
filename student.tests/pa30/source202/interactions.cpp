template<int... I>struct Seq{};
template<int N>struct Build{template<class,int... I>using Map=Seq<I...>;using type=__make_integer_seq<Map,int,N>;};
struct Pair{int value;template<int...I,int...J>Pair(Seq<I...>,Seq<J...>):value(sizeof...(I)+sizeof...(J)){} Pair():Pair(Build<2>::type(),Build<3>::type()) {}};
struct Base{protected:static int value(){return 29;}};
template<class T>struct Mid:Base{};
template<class T>struct Child:Mid<T>{int f(){return Base::value();}};
template<class T>struct Outer{private:static int secret(){return 7;} public: struct Inner{friend class Outer<T>;private:int run(){return Outer<T>::secret();}}; int run(){return Inner().run();}};
struct Complete{template<class T>struct Inner{int get(){return Complete().value;}};int value;Complete():value(3){}};
int destroyed;
struct Guard{Guard(int){} ~Guard(){++destroyed;}};
struct Later{Later(int){throw 13;}};
struct Parts{Guard guard;Later later;};
union Variant{Parts parts;int unused;~Variant(){}};
struct Aggregate{Variant variant;};
int cleanup(){try{Aggregate a={{{7,13}}};}catch(int n){return n;}return 0;}
typedef int Vector __attribute__((vector_size(8)));
volatile Vector stored;
struct Count{int n;operator int()const{return n;}};
template<class T>auto allocate(Count n)->decltype(new T[n]){return new T[n];}
[[noreturn]] void stop(){throw 5;}
int select(int n){if(n ? 2 : 0)return 17;stop();}
int default_lambda(int n=[](){int x=11;return [=](){return x;}();}()){return n;}
int main(){Pair p;Child<int>child;Outer<int>outer;Complete::Inner<int>inner;
int* a=allocate<int>(Count{2});a[1]=default_lambda();auto capture=[&]{return a[1]+p.value;};
Vector v=__builtin_ia32_vec_init_v2si(19,23);stored=v;
int sum=capture()+child.f()+outer.run()+inner.get()+__builtin_ia32_vec_ext_v2si(stored,1)+select(1);
delete[]a;int before=destroyed;int c=cleanup();return sum==95 && c==13 && destroyed==before+1?0:1;}
