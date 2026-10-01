int n=1; static_assert(*(int*)(const int*)&n==1,"nonconstant");
int main(){}
