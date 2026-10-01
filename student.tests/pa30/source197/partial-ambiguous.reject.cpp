template<class...> using discard = void;
template<class A,class B,class C=void> struct choice;
template<class T,class U> struct choice<T*,U,discard<T>> {};
template<class T,class U> struct choice<T,U*,void> {};
choice<int*,char*> value;
