int live;
int trace;
struct Guard {
  int tag;
  Guard(int n):tag(n) { ++live; }
  ~Guard() noexcept(false) { trace=trace*10+tag; --live; }
};
int cell;
int& get(Guard const&) { if(live<1)__builtin_abort(); return cell; }
int check(Guard const&,int value) { if(live!=2)__builtin_abort(); return value; }
int read(Guard const&) { return live; }
struct Base {
  Base(int n) { if(n!=1 || live!=1)__builtin_abort(); }
};
struct Derived:Base {
  Derived():Base(read(Guard(4))) { if(live)__builtin_abort(); }
};
struct Delegated {
  Delegated(int n) { if(n!=1 || live!=1)__builtin_abort(); }
  Delegated():Delegated(read(Guard(5))) { if(live)__builtin_abort(); }
};
int run(bool first) {
  trace=0;
  int n=check(Guard(1),first ? read(Guard(2)) : read(Guard(3)));
  if(n!=2 || live || trace!=(first?21:31))return 1;
  trace=0;
  first ? get(Guard(6)) : get(Guard(7));
  if(live || trace!=(first?6:7))return 2;
  if(read(Guard(8)) && live==1) {} else return 3;
  if(live)return 4;
  switch(read(Guard(9))) {
  case 1: if(live)return 5; break;
  default: return 6;
  }
  Derived derived;
  Delegated delegated;
  return live;
}
int main() { return run(false) || run(true); }
