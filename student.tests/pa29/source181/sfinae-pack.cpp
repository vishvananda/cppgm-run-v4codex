template<int...N> char f(int(*...p)[N]);
template<int...N> long f(...);
static_assert(sizeof(f<1,2>((int(*)[1])0,(int(*)[2])0))==sizeof(char),"positive pack");
static_assert(sizeof(f<1,0>((int(*)[1])0,(int(*)[0])0))==sizeof(long),"zero pack");
int main(){return 0;}
