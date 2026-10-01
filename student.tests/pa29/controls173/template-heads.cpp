template<class T> struct X{T n;};
template<class T>int run(T n){
 auto f=[n]<class U=T, class V=U>(U x=U(2),V y=V(3)){return n+x+y;};
 auto g=[]<template<class>class A,class U>(A<U> a){return a.n;};
 return f()+f(4L,5)+g(X<int>{7});
}
int main(){return run(1)==23 && run(2L)==25?0:1;}
