#include "ownership152.h"
int main() {
  { HostLeaf152 a, b;
    if(!fill152(a,17) || !fill152(b,29)) return 1;
    if(a.value != 17 || b.value != 29) return 2;
  }
  return destroyed152 != 2;
}
