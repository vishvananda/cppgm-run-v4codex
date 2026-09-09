int live;
int copies;
int moves;
struct Value {
  int n;
  Value(int x) : n(x) { ++live; }
  Value(const Value& x) : n(x.n) { ++live; ++copies; }
  Value(Value&& x) : n(x.n) { ++live; ++moves; x.n = -1; }
  ~Value() { --live; }
};
Value make(int n) { return Value(n); }
Value choose(bool b, const Value& fallback) {
  Value local(3);
  return b ? local : fallback;
}
int read(const Value& x) { return x.n; }
bool check(const Value&) { return live != 0; }
bool alive() { return live != 0; }
int run(bool a, bool b) {
  int n = a ? (b ? read(make(5)) : read(make(6))) : read(make(7));
  if (live != 0 || n != (a ? (b ? 5 : 6) : 7)) return 1;
  bool c = (a ? check(make(1)) : check(make(2))) && alive();
  if (!c || live != 0) return 2;
  if (check(make(1)) && alive()) {} else return 3;
  if (live != 0) return 4;
  if (a && check(make(1))) {}
  if (live != 0) return 5;
  {
    const Value& x = make(9);
    if (live != 1 || x.n != 9) return 6;
  }
  if (live != 0) return 7;
  return 0;
}
int main() {
  if (run(false,false) || run(false,true) || run(true,false) || run(true,true)) return 1;
  Value fallback(8);
  Value a = choose(true,fallback);
  Value b = choose(false,fallback);
  return a.n != 3 || b.n != 8 || copies != 2 || moves != 0 || live != 3;
}
