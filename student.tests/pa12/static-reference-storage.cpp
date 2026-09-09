int alive, destroyed, calls;
struct Verify { ~Verify() { if (alive || destroyed != 2) __builtin_abort(); } } verify;
struct A {
  int n;
  A(int x):n(x) { ++alive; }
  ~A() { --alive; ++destroyed; }
};
struct B {
  long pad;
  int n;
  B(int x):pad(0),n(x) { ++alive; }
  ~B() { --alive; ++destroyed; }
};
struct Source { operator A() const { return A(7); } };
struct Number { operator int() const { return ++calls+40; } };
bool select() { return false; }
const A& complete = Source();
const int& member = select() ? A(2).n : B(3).n;
const int& scalar = Number();
const long& converted = Number();
Number number;
const int& from_lvalue = number;
int main() {
  volatile int clobber[4]={6,7,8,9};
  return complete.n != 7 || member != 3 || scalar != 41 || converted != 42 || from_lvalue != 43 ||
    calls != 3 || alive != 2 || destroyed || clobber[3] != 9;
}
