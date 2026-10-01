template<class T,T... I> struct seq {};
typedef __make_integer_seq<seq,int> invalid;
