constexpr int n=1; static_assert(*(volatile int*)&n==1,"volatile access");
int main(){}
