constexpr int n=1; constexpr const void *p=&n; static_assert(*static_cast<const int*>(p)==1,"forbidden");
int main(){}
