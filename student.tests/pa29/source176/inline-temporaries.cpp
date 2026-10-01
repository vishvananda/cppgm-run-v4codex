struct Check { ~Check(); } check;
#include "inline-temporaries.h"
int trace;
int made;
int failed=1;
extern const void* address(int);
extern "C" void _Exit(int);
Check::~Check() { _Exit(!failed && trace==321 ? 0 : 88); }
int main() {
 if(address(0)!=&pair || address(1)!=&pair.a || address(2)!=&pair.b || made!=2) return 1;
 failed=0;
}
