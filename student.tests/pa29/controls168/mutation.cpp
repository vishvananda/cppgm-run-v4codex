struct Pair{int value;int other;};
constexpr int mutate(){Pair p={.value=1};int& r=p.value;r=2;return p.value;}
static_assert(mutate()==2,"aggregate subobject reference mutation");
constexpr int nested(){
 struct Box{Pair p;int a[6];};Box b={.p={.other=7},.a={1}};
 b.p.value=3;b.a[4]=11;int* q=&b.a[2];*q=5;
 Pair copy=b.p;b.p.value=9;
 return copy.value+b.p.value+b.p.other+b.a[4]+b.a[2];
}
static_assert(nested()==35,"sparse nested paths and snapshots");
constexpr int inc(int& x){return ++x;}
constexpr int calls(){Pair p={.value=1};inc(p.value);inc(p.value);return p.value;}
static_assert(calls()==3,"nested-call writes and cache validity");
constexpr int postfix(){Pair p={.value=3};int old=p.value++;p.value*=4;return old+p.value;}
static_assert(postfix()==19,"compound arithmetic");
constexpr int arraycopy(){struct A{int a[8];};A x={};x.a[6]=7;x.a[2]=3;A y=x;x.a[6]=1;return y.a[6]+y.a[2]+y.a[5]+x.a[6];}
static_assert(arraycopy()==11,"split repeated tails only on snapshot");
int main(){return mutate()!=2 || nested()!=35 || calls()!=3 || postfix()!=19 || arraycopy()!=11;}
