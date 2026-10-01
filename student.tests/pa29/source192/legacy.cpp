#include <cmath>
#include <cstdio>
// The captured hosted profile deliberately selects the older header branch.
// Its explicit typedef declarations determine identity in that profile.
#if __GNUC__ < 13
static_assert(__is_same(_Float32,float),"header typedef");
static_assert(__is_same(_Float64,double),"header typedef");
static_assert(__is_same(_Float64x,long double),"header typedef");
#endif
static_assert(sizeof(__float128)==16 && sizeof(_Float16)==2,"builtin formats");
int main(int argc,char**) {
  _Float32 x=argc;
  return std::ceil(x)==1 && (__float128)x==1.0Q ? 0 : 1;
}
