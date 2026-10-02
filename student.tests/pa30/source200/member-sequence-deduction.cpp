template<int... I> struct Seq {};
template<int N> struct Build {
  template<class, int... I> using Map = Seq<I...>;
  using type = __make_integer_seq<Map,int,N>;
};
struct Pair {
  int value;
  template<int... I,int... J> Pair(Seq<I...>,Seq<J...>) : value(sizeof...(I)+sizeof...(J)) {}
  Pair() : Pair(Build<2>::type(),Build<3>::type()) {}
};
int main() { Pair p; return p.value-5; }
