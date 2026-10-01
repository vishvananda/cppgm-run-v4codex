template<int... V> struct Numbers {
 static constexpr int left = (... - V);
 static constexpr int right = (V - ...);
};
static_assert(Numbers<20,3,2>::left==15,"left");
static_assert(Numbers<20,3,2>::right==19,"right");
template<class... T> using Sum = decltype((T() + ...));
static_assert(__is_same(Sum<int,double>,double),"arithmetic type");
template<class... T> using Empty = decltype((T(),...));
static_assert(__is_same(Empty<>,void),"empty comma");
template<class... T> using Last = decltype((static_cast<T&>(*static_cast<T*>(nullptr)),...));
static_assert(__is_same(Last<int,long>,long&),"category");
int main(){return 0;}
