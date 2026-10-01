struct Check { ~Check(); } check;
#include "inline-shared.h"
int calls = 0;
int destroyed = 0;
int failed = 1;
int next() { return ++calls; }
extern const void* other(int);
extern int from_other();
extern "C" void _Exit(int);
Check::~Check() { _Exit(!failed && destroyed == 2 ? 0 : 99); }
int main() {
  if (other(0) != letters || other(1) != &scalar || other(2) != &dynamic) return 1;
  if (other(3) != &object || other(4) != &reference || other(5) != &Members::value) return 2;
  if (other(6) != &TemplateMembers<int>::value || other(7) != &variable<long>) return 3;
  if (other(8) == &private_value) return 4;
  if (other(9) != &tls || tls != from_other()) return 5;
  if (calls != 6 || destroyed != 0 || letters[3] != 't') return 6;
  failed = 0;
  return 0;
}
