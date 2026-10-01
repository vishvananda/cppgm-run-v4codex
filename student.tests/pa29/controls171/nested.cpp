template<class... T> struct list {};
template<unsigned... I> struct indices {};
template<class L,class S> struct project;
template<class... T,unsigned... I> struct project<list<T...>,indices<I...>> {
  template<class U> using types=list<U,__type_pack_element<I,T...>...>;
};
template<class A,class B> struct same { static const bool value=false; };
template<class A> struct same<A,A> { static const bool value=true; };
static_assert(same<project<list<int,long>,indices<1,0>>::types<char>,list<char,long,int>>::value,"two nested packs");
static_assert(same<project<list<>,indices<>>::types<char>,list<char>>::value,"empty expansion does not select an element");
template<class T,T... I> struct seq {};
template<class... T> struct holder {
  template<unsigned N> using generated=__make_integer_seq<seq,unsigned,N>;
  template<unsigned I> using element=__type_pack_element<I,T...>;
};
static_assert(same<holder<int,long>::generated<2>,seq<unsigned,0,1>>::value,"member generator");
static_assert(same<holder<int,long>::element<1>,long>::value,"member selection");
int main(){return 0;}
