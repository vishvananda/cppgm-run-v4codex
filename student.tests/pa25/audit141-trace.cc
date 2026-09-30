struct Left { int x; constexpr Left(int n):x(n){} virtual int left(){return x;} };
struct Right { int y; constexpr Right(int n):y(n){} virtual int right(){return y;} };
template<int N> struct Item:Left,Right {
  Item* self;
  constexpr Item():Left(N),Right(N+1),self(this){}
  int right(){return Right::y+N;}
};
Item<9> first, second;
int destroyed;
struct Guard { ~Guard() noexcept { ++destroyed; } };
template<class T> void relay(T* p,T* replacement) try {
  Guard g; throw p;
} catch(T*& value) {
  value=replacement; throw;
}
int main(){
  try { relay(&first,&second); }
  catch(Right*const& value) {
    if(value!=(Right*)&second || value->right()!=19 || destroyed!=1)return 1;
    if(dynamic_cast<Item<9>*>(value)!=&second || second.self!=&second)return 2;
    try { throw; } catch(Item<9>*const& exact) { if(exact!=&second)return 3; }
    if(value!=(Right*)&second)return 4;
    return 0;
  }
  return 5;
}
