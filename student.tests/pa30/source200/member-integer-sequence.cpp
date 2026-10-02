template<unsigned long... I> struct Seq {};
template<unsigned long N> struct Build {
  template<class, unsigned long... I> using Map = Seq<(N+I)...>;
  using type = __make_integer_seq<Map, unsigned long, N>;
};
template<template<class T,T...> class Target, class T,T N>
struct Apply { using type = __make_integer_seq<Target,T,N>; };
template<class T,T... I> struct TypedSeq {};
static_assert(__is_same(Build<3>::type, Seq<3,4,5>), "member environment");
static_assert(__is_same(Build<0>::type, Seq<>), "empty pack");
static_assert(__is_same(Apply<TypedSeq,int,2>::type, TypedSeq<int,0,1>), "parameter");
int main() { Build<3>::type value; return sizeof(value) != 1; }
