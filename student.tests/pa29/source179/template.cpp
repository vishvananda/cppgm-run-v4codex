template<class T> struct Pair { T first; T second; };
template<class T> int fixed(T n) { auto [a,b]=Pair<int>{2,3}; return a+b+n; }
template<class T> T dependent(Pair<T> p) { auto [a,b]=p; return a+b; }
template<class T> struct Box {
 template<class... A> __attribute__((__visibility__("hidden"))) int call(A&&... a) {
 auto [x,y]=Pair<int>{1,2}; return x+y+sizeof...(a);
 }
};
int main(){ Box<int> b; return fixed(4)!=9 || fixed(2L)!=7 || dependent(Pair<int>{5,6})!=11 || dependent(Pair<long>{7,8})!=15 || b.call(1,2)!=5; }
