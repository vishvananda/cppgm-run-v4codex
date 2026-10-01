template<class T> struct S { static int a[0]; };
template<class T> int S<T>::a[1];
