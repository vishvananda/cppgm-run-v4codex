struct S {int x;constexpr S(int n):x(n){x+=2;}};
constexpr int work(){S s(3);return s.x;}
static_assert(work()==5,"constructor body modifies aggregate");
int main(){return work()!=5;}
