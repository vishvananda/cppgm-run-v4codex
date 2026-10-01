template<class T,T... I> struct seq {static const int size=sizeof...(I);};
template<template<class,int...> class S> struct host {
  typedef S<int,0,1> first;
  typedef S<long,0,1,2> second;
};
static_assert(host<seq>::first::size==2,"dependent non-type head");
static_assert(host<seq>::second::size==3,"actual argument type governs values");
int main(){return 0;}
