template<class T,T... I> struct seq {static const unsigned size=sizeof...(I);};
template<template<class,int...> class S,int N> using make=S<int,__integer_pack(N)...>;
template<class T> struct owner {
  typedef make<seq,0> empty;
  template<int N> using create=make<seq,N>;
};
static_assert(owner<long>::empty::size==0,"empty generator");
static_assert(owner<char>::create<4>::size==4,"dependent nested generator");
template<int... I> int sum(seq<int,I...>,int x){int a[]={0,(I+x)...};int s=0;for(unsigned i=0;i<sizeof...(I)+1;++i)s+=a[i];return s;}
int main(){return sum(make<seq,4>{},3)-18;}
