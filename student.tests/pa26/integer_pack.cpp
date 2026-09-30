template<unsigned long... N> struct Sequence {static const unsigned long count=sizeof...(N);};
template<unsigned long N> struct Build {typedef Sequence<__integer_pack(N)...> type;};
static_assert(__is_same(Build<0>::type,Sequence<>),"empty generator");
static_assert(__is_same(Build<4>::type,Sequence<0,1,2,3>),"ordered generator");
static_assert(Build<1024>::type::count==1024,"large generator");
template<unsigned long N> using More=Sequence<__integer_pack(N+1)...>;
static_assert(__is_same(More<2>,Sequence<0,1,2>),"dependent expression bound");
template<class... T> struct Arity {typedef Sequence<__integer_pack(sizeof...(T))...> type;};
static_assert(__is_same(Arity<int,double>::type,Sequence<0,1>),"pack length bound");
int main(){return 0;}
