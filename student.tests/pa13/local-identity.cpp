struct A { virtual int value() const { return 0; } };
int call(const A& a) { return a.value(); }
int f(int) { struct Local:A{ int value()const override{return 1;} } x; return call(x); }
int f(double) { struct Local:A{ int value()const override{return 2;} } x; return call(x); }
int g() {
  int n=0;
  { struct Local:A{int value()const override{return 3;}} x; n+=call(x); }
  { struct Local:A{int value()const override{return 4;}} x; n+=call(x); }
  return n;
}
int main(){ return f(1)!=1 || f(1.0)!=2 || g()!=7; }
