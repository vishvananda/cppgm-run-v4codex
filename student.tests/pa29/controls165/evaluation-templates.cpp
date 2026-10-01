constexpr bool active(){return __builtin_is_constant_evaluated();}
template<int N> constexpr int choose(int x){return active()?N+x:N-x;}
template<int N> struct State {int x;constexpr State():x(choose<N>(1)){} };
template<int N> int work(int x){
 static_assert(choose<N>(1)==N+1,"");
 constexpr State<N> a;
 const State<N>& b=State<N>();
 const int& c=choose<N>(2);
 int values[]={choose<N>(x),active()};
 constexpr int fixed[]={choose<N>(2),active()};
 return a.x+b.x+c+values[0]+values[1]+fixed[0]+fixed[1];
}
int main(int argc,char**){return work<4>(argc)==26 && work<8>(argc)==46?0:1;}
