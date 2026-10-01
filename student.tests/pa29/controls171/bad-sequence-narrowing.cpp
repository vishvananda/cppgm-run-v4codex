template<class T,T... I> struct seq {};
typedef __make_integer_seq<seq,unsigned char,256> invalid;
