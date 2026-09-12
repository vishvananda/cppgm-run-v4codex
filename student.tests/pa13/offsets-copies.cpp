struct Base { int value; Base(int x):value(x){} };
struct Derived:Base {
  int extra;
  Derived(int x):Base(x),extra(x+1){}
  virtual int sum() const noexcept { return value+extra; }
};
int get(Base* p) { return p ? p->value : 0; }
int use(Derived* d) { Base* b=d; return get(b); }
int main() {
  Derived d(17); Derived copy=d; Derived assigned(0); assigned=copy;
  Base& base=assigned;
  if(copy.sum()!=35 || assigned.sum()!=35 || base.value!=17) return 1;
  return use(&d)!=17 || use(0)!=0;
}
