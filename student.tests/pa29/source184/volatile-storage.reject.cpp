constexpr volatile int n=1; static_assert(*(int*)&n==1,"volatile");
int main(){}
