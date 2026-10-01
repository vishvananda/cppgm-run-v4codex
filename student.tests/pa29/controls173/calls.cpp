template<int... I> struct Seq {};
template<class V> int outer(V v) {
  auto call = [&]<class T>(T x, int n=3) -> int { v+=x; return v+n; };
  return call(2)+call(3L);
}
int main() {
  auto id=[]<class T>(T x){return x;};
  if(id(4)!=4 || id(7L)!=7) return 1;
  auto explicit_call=[]<int N, class T>(T x){return N+x;};
  if(explicit_call.operator()<4>(3)!=7) return 2;
  auto count=[]<int...I>(Seq<I...>){return sizeof...(I);};
  if(count(Seq<1,2,3>{})!=3 || count(Seq<>{})!=0) return 3;
  if(outer(1)!=15 || outer(2L)!=17) return 4;
  auto sum=[]<class... T>(T... v){return (v+...+0);};
  if(sum(1,2,3)!=6 || sum()!=0) return 5;
  return 0;
}
