constexpr bool active(){return __builtin_is_constant_evaluated();}
constexpr int aliases(){int&& r=active()?3:7;r=11;const int& s=r;return s;}
static_assert(aliases()==11,"");
int main(){
 int&& r=active()?3:7;r=11;const int& s=r;
 bool a[]={s==11};
 int other=19;const int& selected=active()?r:other;
 constexpr int folded=aliases();
 int b[]={aliases()};
 return s==11&&selected==11&&a[0]&&folded==11&&b[0]==11?0:1;
}
