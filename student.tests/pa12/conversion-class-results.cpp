int copies;
int alive;
struct Value {
  int n;
  Value(int x):n(x) { ++alive; }
  Value(const Value& x):n(x.n) { ++copies; ++alive; }
  ~Value() { --alive; }
};
struct Reference {
  Value* p;
  Reference(Value& v):p(&v) {}
  operator Value&() const { return *p; }
};
struct Prvalue {
  operator Value() const { return Value(9); }
};
Value copy(Reference r) { return r; }
int take(Value v) { return alive == 2 && v.n == 7; }
int main() {
  {
    Value v(7); Reference r(v);
    if (!take(r) || copies != 1 || alive != 1) return 1;
    Value w = copy(r);
    if (copies != 2 || alive != 2 || w.n != 7) return 2;
    Prvalue p;
    const Value& x = static_cast<Value>(p);
    if (x.n != 9 || alive != 3) return 3;
  }
  return alive;
}
