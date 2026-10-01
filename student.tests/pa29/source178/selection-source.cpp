int calls, dead;
constexpr int site(int n=__builtin_LINE()) { return n; }
struct Guard { int line; Guard(int n=site()):line(n) { ++calls; } ~Guard(){++dead;} };
template<int N> int work() {
#line 100 "selection.cpp"
  if constexpr (constexpr int n=site(); N) {
    if (Guard g; g.line!=101) return 1;
    return n==100 ? 0 : 2;
  } else return 3;
}
int main() { return work<1>()==0 && calls==1 && dead==1 ? 0 : 1; }
