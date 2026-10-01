int count;
struct Guard { Guard(){++count;} ~Guard(){--count;} };
template<class T> int run(T n) {
  if constexpr (Guard guard; sizeof(T)==sizeof(int)) {
    if constexpr (using U=T; sizeof(U)==4) {
      switch(int x=n; x) {case 2:return count+x; default:break;}
    } else return n.absent();
  } else return n.invalid();
  return count;
}
int main() { return run(2)==3 && count==0 && run(0)==0 ? 0 : 1; }
