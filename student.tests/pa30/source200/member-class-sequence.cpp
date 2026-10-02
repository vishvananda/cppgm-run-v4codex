struct Concrete { template<class T,T... I> struct Seq { static const int count=sizeof...(I); }; };
template<class U> struct Family { template<class T,T... I> struct Seq { using type=U; static const int count=sizeof...(I); }; };
using A=__make_integer_seq<Concrete::Seq,int,3>;
using B=__make_integer_seq<Family<char>::Seq,int,4>;
static_assert(A::count==3 && B::count==4,"member class");
static_assert(__is_same(B::type,char),"enclosing environment");
int main(){ return A::count+B::count-7; }
