struct S{const int x;};constexpr int f(){S s={.x=1};int& r=const_cast<int&>(s.x);r=2;return s.x;}static_assert(f()==2,"");
