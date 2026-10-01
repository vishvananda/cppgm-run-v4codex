int x=1;constexpr int f(){x=2;return x;}static_assert(f()==2,"");
