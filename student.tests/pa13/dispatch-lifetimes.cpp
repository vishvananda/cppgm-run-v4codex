int trace;
struct Base {
  Base() { trace = trace*10 + value(); }
  virtual int value() const noexcept { return 1; }
  virtual ~Base() noexcept { trace = trace*10 + value(); }
};
struct Derived : Base {
  Derived() { trace = trace*10 + value(); }
  int value() const noexcept override { return 2; }
  ~Derived() noexcept override { trace = trace*10 + value(); }
};
int inspect(const Base& b) { return b.value(); }
int main() {
  { Derived d; Base* p = &d; if(p->value()!=2 || inspect(d)!=2 || d.Base::value()!=1) return 1; }
  if(trace!=1221) return 2;
  trace=0;
  { const Base& b=Derived(); if(b.value()!=2) return 3; }
  return trace!=1221;
}
