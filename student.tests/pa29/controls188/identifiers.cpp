namespace funcs {
int co_await(int x) { return x+1; }
int co_yield(int x) { return x+2; }
int co_return(int x) { return x+3; }
}
struct Late {
  int value() { return co_await + co_yield(2) + co_return; }
  int co_await;
  int co_yield(int x) { return x*3; }
  enum { co_return=4 };
};
struct Base { int co_await() { return 11; } };
struct Derived : Base { int value() { return co_await(); } };
template<class T> int local(T co_await) {
  int co_yield = co_await + 2;
  int co_return = co_yield + 3;
  return co_return;
}
template<class co_await> int type_name() { co_await value = co_await(7); return value; }
int labels() { goto co_return; co_await: return 2; co_yield: return 3; co_return: return 5; }
// Complete-class lookup sees later lists, enums and parenthesized declarators.
struct LateList {
  int value() { return co_await+co_yield(2)+co_return; }
  int a, co_await;
  int b, co_yield(int);
  int c, co_return;
};
int LateList::co_yield(int x) { return x; }
struct LateEnum {
  int value() { return co_await+co_yield+co_return; }
  enum { a, co_await=2, b, co_yield=3, c, co_return=4 };
};
struct LatePointer {
  int value() { return co_await(2)+co_yield[0]+co_return; }
  int (*co_await)(int);
  int (co_yield)[1];
  int (co_return);
};
template<class T> struct Result { int n; };
struct LateQualified {
  int value() { return co_await().n; }
  Result<int> co_await() { return Result<int>{12}; }
};
int late_cases() {
  LateList a; a.co_await=3; a.co_return=4;
  LateEnum b; LatePointer c; c.co_await=funcs::co_await; c.co_yield[0]=4; c.co_return=5;
  LateQualified d;
  return a.value()==9 && b.value()==9 && c.value()==12 && d.value()==12;
}
int main() {
  Late l; l.co_await=5; Derived d;
  using funcs::co_await; using funcs::co_yield; using funcs::co_return;
  return late_cases() && l.value()==15 && d.value()==11 && local(2)==7 && type_name<int>()==7 && labels()==5 &&
    co_await(1)==2 && co_yield(1)==3 && co_return(1)==4 ? 0 : 1;
}
