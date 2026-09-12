struct A {
  virtual int f(int n) const { return n; }
  virtual int f(double n) const { return int(n)+1; }
  virtual A* self() { return this; }
};
struct B:A {
  int f(int n) const override { return n+7; }
  int f(double n) const override { return int(n)+8; }
  B* self() override { return this; }
};
struct C:B { int f(int n) const override final { return n+9; } };
int run(A& a) { return a.f(3)+a.f(4.0); }
int main(){ C c; A& a=c; return run(a)!=24 || a.self()!=&c || c.A::f(3)!=3; }
