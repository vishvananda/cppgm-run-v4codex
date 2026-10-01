constexpr int n=1; static_assert(*((int*)&n+1)==1,"past end");
int main(){}
