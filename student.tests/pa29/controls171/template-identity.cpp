template<template<unsigned long,class...> class Select> struct selection {
  typedef Select<1,char,int> type;
};
selection<__type_pack_element>::type value=3;
template<class T,T... I> struct seq {static const unsigned size=sizeof...(I);};
template<template<template<class V,V...> class,class T,T> class Make> struct generate {
  typedef Make<seq,int,3> type;
};
static_assert(generate<__make_integer_seq>::type::size==3,"sequence template identity");
int main(){::__type_pack_element<0,int> n=3;return value-n;}
