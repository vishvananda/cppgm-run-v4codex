#include "attributes151.h"
extern int use_effects(int);
int main() {
  Record r(13);
  Record* p = &r;
  return p->read() == 13 && tagged(9) == 11 && versioned == 7 &&
    use_effects(3) == 15 ? 0 : 1;
}
