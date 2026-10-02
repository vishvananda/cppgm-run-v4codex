template<unsigned long N> struct Build {
  template<class, unsigned long...> using Map = int;
  using type = __make_integer_seq<Map, int, -1>;
};
Build<2>::type value;
