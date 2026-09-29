struct S {
  int value;
  int get() const { return value; }
  int add(int x) const { return value+x; }
};
int call0(int (S::*p)() const, const S& s);
int call1(int (S::*p)(int) const, const S& s);
