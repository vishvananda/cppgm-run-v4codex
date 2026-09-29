#include "adjustment.h"
struct Pad { int pad; };
struct D : Pad, S {
  int extra;
  int get_extra() const { return pad; }
  int add_extra(int x) const { return pad+x; }
};
int main() {
  D d; d.pad=7; d.value=3; d.extra=7;
  return call0(static_cast<int(S::*)()const>(&D::get_extra),d)!=7 ||
    call1(static_cast<int(S::*)(int)const>(&D::add_extra),d)!=12;
}
