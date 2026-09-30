struct Left { int x; constexpr Left(int n):x(n){} virtual int left(){return x;} };
struct Right { int y; constexpr Right(int n):y(n){} virtual int right(){return y;} };
template<int N> struct Item:Left,Right {
  Item* self;
  constexpr Item():Left(N),Right(N+1),self(this){}
  int left(){return Left::x+100;}
  int right(){return Right::y+200;}
};
Item<9> item;
int main(){
  Right* r=&item; Left* l=&item;
  if(item.self!=&item || l->left()!=109 || r->right()!=210)return 1;
  if(dynamic_cast<Item<9>*>(r)!=&item || dynamic_cast<Left*>(r)!=l)return 2;
  return 0;
}
