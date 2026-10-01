struct outer {template<class T> struct box {}; box(int)->box<int>;
template<class T> box(T*)->box<T*>;};
template<class X> struct envelope {template<class T> struct box {}; box(X)->box<X>;
template<class U> box(U*)->box<U*>;};
int main(){envelope<int> a; envelope<char> b; outer::box<int> c;return sizeof(a)!=sizeof(b)||sizeof(c)!=1;}
