enum E {two=2};
template<class T,T... I> struct seq {};
typedef __make_integer_seq<seq,E,two> invalid;
