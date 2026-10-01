namespace [[deprecated("test")]] sample __attribute__((visibility("default"))) { int x=2; }
extern "C" int strcmp(const char* _Nonnull, const char* _Nullable);
template<class T> int run() {
    int result=0;
    if ([[maybe_unused]] int n = sizeof(T)) result += n;
    for ([[maybe_unused]] int i=0;i<3;++i) result+=i;
    int xs[2]={5,7};
    for ([[maybe_unused]] int x : xs) result+=x;
    return result;
}
template<int N> struct __attribute__((aligned(N))) Aligned { char x; };
template<class T> struct Holder { char first; __attribute__((aligned(alignof(T)))) char x; };
struct __attribute__((aligned(1))) Natural { long x; };
struct __attribute__((aligned)) Maximum { char x; };
struct Post { char c; int x __attribute__((aligned(32))); };
static_assert(alignof(Aligned<16>)==16 && sizeof(Aligned<32>)==32,"template class");
static_assert(alignof(Holder<long>)==8 && sizeof(Holder<char>)==2,"dependent field");
static_assert(alignof(Natural)==8 && alignof(Maximum)==16,"GNU minimum alignment");
static_assert(alignof(Post)==32 && sizeof(Post)==64,"declarator attribute");
__attribute__((aligned(32))) int global=9;
struct __attribute__((unknown(aligned(64)))) Ignored { char x; };
struct Foreign { int x; [[other::no_unique_address]] struct E{} e; };
static_assert(alignof(Ignored)==1 && sizeof(Foreign)==8,"unknown attributes stay inert");
struct Suffix { char x; } __attribute__((aligned(32)));
struct __attribute__((aligned(8))) Redeclared;
struct __attribute__((aligned(16))) Redeclared {};
static_assert(alignof(Suffix)==32 && alignof(Redeclared)==16,"suffix and merged GNU alignment");
typedef __int128 NaturalWide __attribute__((aligned(16)));
static_assert(alignof(NaturalWide)==16 && __is_same(NaturalWide,__int128),"redundant typedef alignment");
int main(){ __attribute__((aligned(32))) int local=7;
 return run<int>()!=19 || ((unsigned long)&local%32) || ((unsigned long)&global%32) || strcmp("a","a"); }
