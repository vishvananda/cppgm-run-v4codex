template<class A,class B> struct same { static const bool value=false; };
template<class A> struct same<A,A> { static const bool value=true; };
template<class T,T... I> struct seq { static const unsigned size=sizeof...(I); };
template<class T,T N> using make=__make_integer_seq<seq,T,N>;
template<template<class V,V...> class S,class T,T N> using indirect=__make_integer_seq<S,T,N>;
template<class T,T... I> using alias=seq<T,I...>;
static_assert(same<make<int,0>,seq<int>>::value,"zero count");
static_assert(same<make<unsigned,4>,seq<unsigned,0,1,2,3>>::value,"integer values and type");
static_assert(same<indirect<seq,short,3>,seq<short,0,1,2>>::value,"template argument");
static_assert(same<__make_integer_seq<alias,long,2>,seq<long,0,1>>::value,"alias target");
static_assert(same<__make_integer_seq<seq,unsigned,(unsigned)3ULL>,seq<unsigned,0,1,2>>::value,"cast count");
template<int N> auto choose(int)->__make_integer_seq<seq,int,N>;
template<int N> long choose(...);
static_assert(same<decltype(choose<-1>(0)),long>::value,"negative count SFINAE");
template<int... I> int sum(seq<int,I...>,int x) { int a[]={0,(x+I)...}; int s=0;for(unsigned i=0;i<sizeof...(I)+1;++i)s+=a[i];return s; }
int main(){return sum(make<int,4>{},3)-18;}
