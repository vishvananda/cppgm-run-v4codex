// Parameter destruction may occur at callee exit or after the call. Track a
// completed RAII subobject rather than equating constructor-body side effects
// with completed construction of the containing object. See lifetime179.md.
int live,temps,dead,throw_default;
struct Token{Token(){++temps;}~Token() noexcept(false){--temps;if(throw_default && ++dead==throw_default)throw 7;}};
struct Guard{Guard(){++live;}Guard(const Guard&){++live;}~Guard(){--live;}};
struct Item{Guard g;int a;Item():a(2){}Item(const Item& x,Token=Token()):g(x.g),a(x.a){}~Item(){}};
int work(){Item a[10];throw_default=6;try{auto [b,c,d,e,f,g,h,i,j,k]=a;return 1;}catch(int x){if(x!=7||temps||live!=10)return 2;}throw_default=0;return 0;}
int main(){return work()||live||temps;}
