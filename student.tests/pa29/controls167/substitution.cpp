template<unsigned N> using Vec __attribute__((ext_vector_type(N))) = int;
template<unsigned N> auto pick(int) -> decltype(sizeof(Vec<N>),int()) {return 1;}
template<unsigned N> long pick(...) {return 2;}
static_assert(__is_same(decltype(pick<0>(0)),long),"invalid substituted width is SFINAE");
static_assert(__is_same(decltype(pick<3>(0)),int),"valid substituted width");
template<class... T> struct List {};
template<unsigned... N> struct Shapes {using type=List<Vec<N>...>;};
static_assert(__is_same(Shapes<2,3,4>::type,List<Vec<2>,Vec<3>,Vec<4>>),"dependent vector pack");
int main(){return pick<0>(0)==2 && pick<3>(0)==1?0:1;}
