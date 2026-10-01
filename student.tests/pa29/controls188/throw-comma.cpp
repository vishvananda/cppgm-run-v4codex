int effect;
struct Guard { ~Guard() { ++effect; } };
int wrong() { effect += 100; return 1; }
int f(int x) { Guard g; return (throw x, wrong()); }
int g(int x) { Guard guard; throw x, wrong(); }
int h(int x) { try { throw x; } catch (...) { throw, wrong(); } }
template<class T> int templ(T x) { return (throw x, wrong()); }
int main() {
  int sum=0;
  try { f(3); } catch(int n) { sum+=n; }
  try { g(4); } catch(int n) { sum+=n; }
  try { h(5); } catch(int n) { sum+=n; }
  try { templ(6); } catch(int n) { sum+=n; }
  return sum==18 && effect==2 ? 0 : 1;
}
