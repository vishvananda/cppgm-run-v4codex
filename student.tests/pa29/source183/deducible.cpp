template<class T> struct box {};
template<class U> struct wrap {};
template<class... U> struct list {};
template<template<class> class C, class U> box(C<U>)->box<U>;
template<class... U> box(list<U...>)->box<int>;
template<class C, class U> box(U C::*)->box<U>;
template<class U, int N> box(U(&)[N])->box<U>;
template<class U, class V=void> box(U, int)->box<V>;
int main(){return 0;}
