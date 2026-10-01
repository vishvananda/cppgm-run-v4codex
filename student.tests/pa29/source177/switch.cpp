int live;
struct Guard {
  int value;
  Guard(int n) : value(n) { ++live; }
  ~Guard() { --live; }
  operator int() const { return value; }
};
int go(int n) {
  switch (Guard g(n); n) {
  case 1: if (live!=1) return 10; break;
  case 2: if (live!=1) return 11;
  }
  if (live) return 12;
  switch (Guard g(n); Guard h{n}) {
  case 1: if (live!=2) return 13; break;
  case 2: if (live!=2) return 14;
  }
  if (live) return 15;
  switch (Guard g{n}) { case 1: break; }
  return live;
}
template<class T> int run(T n) {
  switch (using V=T; V value=n) { case 3: return value; default: return 0; }
}
int main() {
  for (int i=0;i<4;++i) { int r=go(i); if(r)return r; }
  if (run(3)!=3) return 16;
  try { switch (Guard g(1); 1) { case 1: throw 2; } }
  catch (int) { if(live)return 17; }
  return 0;
}
