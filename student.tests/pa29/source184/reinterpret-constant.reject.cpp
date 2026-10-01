constexpr int n=1; static_assert(*reinterpret_cast<const int*>(&n)==1,"forbidden");
int main(){}
