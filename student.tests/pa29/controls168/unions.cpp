union U { int a; long b; };
struct S { int tag; U u; int end; };
constexpr S global={.u={.b=19}};
static_assert(global.tag==0 && global.u.b==19 && global.end==0,"union selection");
long read(long x){return ((S){.u={.b=x}}).u.b;}
int main(){U u={.b=17}; S s={2,{.b=13},4};return u.b!=17 || s.u.b!=13 || read(91)!=91;}
