template<class X> struct outer {
 template<class T> struct box {};
 explicit(sizeof(X)>4) box(X x,int n=sizeof(x))->box<X>;
 template<class T> explicit(sizeof(T)>sizeof(X)) box(T* p,int n=sizeof(*p))->box<T>;
};
int main(){outer<int> a;outer<long> b;return sizeof(a)!=sizeof(b);}
