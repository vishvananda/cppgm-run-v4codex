int calls;
int step() { return ++calls; }
struct Guard {
  int* live;
  Guard(int& n) : live(&n) { ++*live; }
  ~Guard() { --*live; }
  explicit operator bool() const { return true; }
};
int exit_return(int& live) {
  if (Guard g(live); g) return live;
  return -1;
}
int main() {
  int live=0, sum=0;
  if (int a=step(), b=step(); a<b) sum += a+b;
  if (; false) return 1;
  if (step(); true) sum += calls;
  if (using Number=int; Number value=4) sum += value;
  if (Guard a(live); Guard b{live}) { if (live!=2) return 2; }
  else return 3;
  if (live || sum!=10 || calls!=3) return 4;
  if (exit_return(live)!=1 || live) return 5;
  for (int i=0;i<5;++i) {
    if (Guard g(live); i==1) continue;
    else if (i==3) break;
    if (live) return 6;
  }
  if (live) return 7;
  if (Guard g(live); true) goto done;
done:
  if (live) return 8;
  try { if (Guard g(live); true) throw 5; }
  catch (int n) { if (n!=5 || live) return 9; }
  return live;
}
