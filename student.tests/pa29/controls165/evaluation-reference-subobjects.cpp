constexpr bool active(){return __builtin_is_constant_evaluated();}
struct S{int a,b;};
int main(){ S s{3,5};int ar[3]={2,4,6};
const int& a=active()?s.a:s.b;
const int& b=active()?ar[2]:ar[0];
return &a==&s.a&&&b==&ar[2]?0:1;}
