int moves;
struct Item{int v;Item(int x):v(x){}Item(const Item&)=delete;Item(Item&& other):v(other.v){other.v=0;++moves;}};
int main(){Item a[2]={Item(3),Item(4)};int before=moves;auto [x,y]=static_cast<Item(&&)[2]>(a);return moves-before!=2||x.v+y.v!=7||a[0].v||a[1].v;}
