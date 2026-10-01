template<class A, class B> struct same { static const bool value = false; };
template<class A> struct same<A,A> { static const bool value = true; };
template<unsigned I, class... T> using at = __type_pack_element<I,T...>;
template<class T> using fixed = __type_pack_element<0,int,T>;
typedef int F(double);
static_assert(same<at<0,const int&,void,F>,const int&>::value,"reference/cv identity");
static_assert(same<at<1,const int&,void,F>,void>::value,"void identity");
static_assert(same<at<2,const int&,void,F>,F>::value,"function identity");
static_assert(same<at<0,int[3]>,int[3]>::value,"array identity");
static_assert(same<fixed<long>,int>::value,"fixed prefix of a dependent query");
template<unsigned I,class... T> at<I,T...>* selected(T*...);
template<class T> auto fallback(int)->__type_pack_element<3,T,int>;
template<class T> long fallback(...);
static_assert(same<decltype(fallback<char>(0)),long>::value,"out of range SFINAE");
int main() {
  ::__type_pack_element<0,int> a = __type_pack_element<1,char,int>(7);
  at<1,char,int>& b=a;
  b+=3;
  return a-10;
}
