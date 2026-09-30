int trace;
struct Value {
  int number;
  Value(int n) : number(n) { if(n == 11) throw 2; }
  ~Value() { trace = trace*10 + number; }
};
int read(Value const&) { throw 1; }
int choose() {
  try { return read(Value(7)); }
  catch(...) { return read(Value(11)); }
}
int main() {
  try { choose(); }
  catch(int n) { return n == 2 && trace == 7 ? 0 : 1; }
  return 2;
}
