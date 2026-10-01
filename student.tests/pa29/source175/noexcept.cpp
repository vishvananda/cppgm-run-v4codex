extern "C" int printf(const char*,...);
constexpr int loc(int n=__builtin_LINE()) noexcept {return n;}
int intrinsic_only() noexcept { return __builtin_LINE(); }
static_assert(noexcept(__builtin_FILE()),"file nonthrowing");
static_assert(noexcept(__builtin_LINE()),"line nonthrowing");
static_assert(noexcept(__builtin_FUNCTION()),"function nonthrowing");
int main(){printf("%d %d\n",intrinsic_only(),loc());}
