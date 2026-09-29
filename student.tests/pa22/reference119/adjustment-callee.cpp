#include "adjustment.h"
int call0(int (S::*p)() const, const S& s) { return (s.*p)(); }
int call1(int (S::*p)(int) const, const S& s) { return (s.*p)(5); }
