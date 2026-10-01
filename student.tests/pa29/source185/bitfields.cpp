struct Small { unsigned _BitInt(7) u:3; _BitInt(7) s:4; };
constexpr Small c={7,-4};
static_assert(__is_same(decltype(+c.u),unsigned _BitInt(7)),"bitint bitfield promotion");
static_assert(c.u==7 && c.s==-4,"bitfield constants");
struct Wide { unsigned _BitInt(93) a:67; _BitInt(93) b:71; };
int main() {
 Small s={7,-4}; ++s.u; s.s+=2;
 if(s.u!=0 || s.s!=-2) return 1;
 Wide w={(static_cast<unsigned _BitInt(93)>(1)<<66)+3,-5};
 if(w.a>>66!=1 || w.b!=-5) return 2;
 return 0;
}
