using Zero = int[0];
using Unknown = int[];
using ConstZero = const int[0];
static_assert(sizeof(Zero) == 0 && alignof(Zero) == alignof(int), "layout");
static_assert(!__is_same(Zero,Unknown), "identity");
static_assert(__is_same(__remove_const(ConstZero),Zero), "cv preservation");
static_assert(__is_same(const Zero,ConstZero), "qualify");
extern int a[];
int a[0];
static_assert(sizeof(a) == 0, "composite declaration");
int which(int (*)[0]) { return 1; }
int which(int (*)[]) { return 2; }
int main() { Zero z{}; Unknown* u = 0; return which(&z) != 1 || which(u) != 2; }
