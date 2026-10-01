struct E{}; struct H{[[no_unique_address]] E e;};
typedef long Low __attribute__((aligned(1)));
typedef int High __attribute__((aligned(32)));
struct HostS{char c; Low x; [[no_unique_address]] H e;};
struct HostT{char c; High x;};
struct HostU{char c; H h; [[no_unique_address]] E e;};
extern "C" int update(HostS*,HostT*,HostU*);
extern "C" int verify(HostS*,HostT*,HostU*);
extern "C" int owned();
