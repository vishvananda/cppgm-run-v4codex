struct Pair {int a;int& b;};
constexpr int compute(){int n=2;auto [x,y]=Pair{3,n};y=8;return x+y+n;}
static_assert(compute()==19,"constant references");
int main(){return compute()!=19;}
