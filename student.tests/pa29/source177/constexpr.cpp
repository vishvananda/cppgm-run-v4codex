template<class T> struct Missing;
int live;
struct Guard { Guard() { ++live; } ~Guard() { --live; } };
template<class T> int choose(T value) {
  if constexpr (using A=T; sizeof(A)==sizeof(int)) return value+live;
  else { using X=typename Missing<T>::type; return X::missing; }
}
template<class T> int initialized(T value) {
  if constexpr (Guard g; sizeof(T)==sizeof(int)) return choose(value);
  else return value.no_such_member();
}
constexpr int sum(int n) {
  int result=0;
  for(int i=0;i<n;++i) {
    if (int x=i; x%2) result+=x;
    else result+=2*x;
  }
  return result;
}
static_assert(sum(5)==16,"selection initialization during constant execution");
int main() {
  if (initialized(4)!=5 || live) return 1;
  if constexpr (constexpr int x=3; x==3) { if(x!=3)return 2; }
  if constexpr (using A=int; sizeof(A)==4) return 0;
  return 3;
}
