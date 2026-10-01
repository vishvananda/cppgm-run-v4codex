struct Z { alignas(64) int a[0]; };
struct S { char x; Z z; char y; };
static_assert(sizeof(Z)==0 && alignof(Z)==64,"zero alignment");
static_assert(sizeof(S)==128 && __builtin_offsetof(S,z)==64 && __builtin_offsetof(S,y)==64,"alignment extent");
int main(){S s{3,{},8};return s.x!=3||s.y!=8;}
