template<int N> constexpr int width(_BitInt(N)) { return N; }
template<unsigned N> constexpr int uwidth(unsigned _BitInt(N)) { return N; }
template<int N> constexpr int expression_width(_BitInt(N+1)) { return N+1; }
template<class...Ts> struct types { static const int count = sizeof...(Ts); };
template<int...Ns> using bitints = types<_BitInt(Ns)...>;
static_assert(bitints<3,7,19>::count==3,"type width pack");
template<int...Ns> constexpr int count(_BitInt(Ns)... x) { return sizeof...(Ns); }
static_assert(width((_BitInt(7))0)==7,"deduced width");
static_assert(uwidth((unsigned _BitInt(93))0)==93,"unsigned deduced width");
static_assert(count((_BitInt(2))0,(_BitInt(7))0)==2,"deduced pack");
static_assert(expression_width<6>((_BitInt(7))0)==7,"width expression");
template<class T> struct traits { static const int width=0; };
template<int N> struct traits<_BitInt(N)> { static const int width=N; };
static_assert(traits<_BitInt(13)>::width==13,"partial specialization");
int main(){return width((_BitInt(7))0)!=7;}
