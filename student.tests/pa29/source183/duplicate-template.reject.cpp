template<class T> struct box {};
template<class X> box(X)->box<X>;template<class Y> box(Y)->box<Y>;
