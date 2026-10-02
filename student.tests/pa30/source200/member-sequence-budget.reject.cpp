template<class U> struct Family { template<class T,T... I> struct Seq {}; };
using Huge=__make_integer_seq<Family<int>::Seq,int,1048577>;
