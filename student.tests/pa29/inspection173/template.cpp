template<class T>struct Value{T n;};
inline int templates(int x){auto f=[]<template<class>class C,class T>(C<T> a)->int{return a.n;};return f(Value<int>{x});}
int materialize(int x){return templates(x);}
