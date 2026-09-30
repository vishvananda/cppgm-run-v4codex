using U=unsigned __int128;
using I=__int128;
constexpr U high=U(1)<<90;
struct Bits { U a:100; U b:20; I c:80; };
Bits g = {high+7,123,-123};
constexpr Bits k={high+3,99,-7};
static_assert(k.a==high+3 && k.b==99 && k.c==-7,"constant fields");
int main(int argc,char**) {
  if(g.a!=high+7 || g.b!=123 || g.c!=-123) return 1;
  Bits local={high+argc,123,-23};
  if(local.a!=high+1 || local.b!=123 || local.c!=-23) return 2;
  local.a=high+9; local.b=987; local.c=-37;
  if(local.a!=high+9 || local.b!=987 || local.c!=-37) return 3;
  return 0;
}
