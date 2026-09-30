namespace std {
typedef void (*unexpected_handler)();
unexpected_handler set_unexpected(unexpected_handler) throw();
}
extern "C" void abort();
int live, destroyed, converted, selected;
struct Guard {
  int id;
  Guard(int n) : id(n) { live += n; }
  ~Guard() { live -= id; destroyed = destroyed*10 + id; }
};
void unexpected_value() { if(live) abort(); ++converted; throw 2.5; }
void constrained(int n) throw(int, double);
void constrained(int n) throw(double, const int, int) {
  Guard g(1);
  if(n == 0) throw 7;
  if(n == 1) throw 8.0;
  try { Guard inner(2); throw 'x'; }
  catch(char) { ++selected; throw; }
}
template<class T> void template_spec() throw(T) { Guard g(3); throw 4L; }
struct Base { int n; Base() : n(11) {} };
struct Derived : Base {};
void allowed_derived() throw(Base) { throw Derived(); }
void nested() {
  Guard a(1);
  try { Guard b(2); throw 7; }
  catch(int) { Guard c(3); try { throw 8; } catch(...) { throw; } }
}
int main() {
  std::set_unexpected(unexpected_value);
  try { constrained(0); } catch(int n) { if(n != 7 || live) return 1; }
  try { constrained(1); } catch(double n) { if(n != 8.0 || live) return 2; }
  destroyed = 0;
  try { constrained(2); } catch(double n) { if(n != 2.5 || live || destroyed != 21) return 3; }
  try { template_spec<double>(); } catch(double n) { if(n != 2.5 || live) return 4; }
  try { allowed_derived(); } catch(Base& b) { if(b.n != 11) return 5; }
  destroyed = 0;
  try { nested(); } catch(int n) { if(n != 8 || live || destroyed != 231) return 6; }
  return converted == 2 && selected == 1 ? 0 : 7;
}
