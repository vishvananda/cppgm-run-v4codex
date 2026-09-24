
template<class T>T&& val();
template<bool,class T=void>struct enable{};
template<class T>struct enable<true,T>{using type=T;};
template<class S,class T,class=void>struct streams{static const bool ok=false;};
template<class S,class T>struct streams<S,T,decltype(val<S&>() << val<T const&>(),void())>{static const bool ok=true;};
template<class S,class T,typename enable<streams<S,T>::ok,int>::type=0>
S&& operator<<(S&& s,T const& t){s<<t;return static_cast<S&&>(s);}

 struct sink{int n;};struct value{enum E{zero,one};E n;operator E()const{return n;}};
 sink&operator<<(sink&s,value v){s.n+=static_cast<int>(v.n);return s;}
 template<class T>int dispatch(T const&v){sink s;s.n=0;s<<v;s<<v;return s.n;}
int main(){value v={value::one};return dispatch(v)!=2;}
