int live,temps,dead,throw_default;
struct Token{Token(){++temps;}~Token() noexcept(false){--temps;if(throw_default && ++dead==throw_default)throw 7;}};
struct Item{int a;Item():a(2){++live;}Item(const Item& x,const Token& = Token()):a(x.a){++live;}~Item(){--live;}};
int work(){Item a[10];throw_default=6;try{auto [b,c,d,e,f,g,h,i,j,k]=a;return 1;}catch(int x){if(x!=7||temps||live!=10)return 2;}throw_default=0;return 0;}
int main(){return work()||live||temps;}
