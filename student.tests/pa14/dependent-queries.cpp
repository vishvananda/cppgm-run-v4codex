template<class A,class B> auto difference(const A& left,const B& right)->decltype(right-left);
template<class X,class Y> auto difference(const X& x,const Y& y)->decltype(y-x) { return y-x; }
template<class T> auto reference(T& x)->decltype((x)) { return x; }
template<class T> auto first(T (&a)[3])->decltype(&a[0]) { return &a[0]; }
template<class T> T&& query_value();
namespace lookup {
struct Key {
    int value;
    friend long hidden(const Key& key) { return key.value+4; }
};
}
template<class T> auto associated(T& x)->decltype(hidden(x)) { return hidden(x); }
template<class T> auto indirect(T&& fn)->decltype(query_value<T>()()) { return fn(); }
int answer() { return 19; }
namespace array_query { template<class T> T* begin(T&); }
template<class T> char (&array_result(T* p,decltype(array_query::begin(*p))*))[2];
int main() {
    if (difference(3,8L)!=5 || sizeof(difference(3,8L))!=sizeof(long)) return 1;
    int n=2;
    reference(n)=7;
    if (n!=7 || &reference(n)!=&n) return 2;
    int a[3]={4,5,6};
    if (first(a)!=&a[0] || *first(a)!=4) return 3;
    lookup::Key key={6};
    if (associated(key)!=10 || sizeof(associated(key))!=sizeof(long)) return 4;
    int (*fn)()=answer;
    if (indirect(fn)!=19) return 5;
    return sizeof(array_result((int*)0,(int**)0))!=2;
}
