constexpr int n=1; constexpr int change(){return ++*(int*)&n;} static_assert(change()==2,"write");
int main(){}
