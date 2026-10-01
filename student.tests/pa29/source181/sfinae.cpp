template<int N> char f(int(*)[N]);
template<int N> long f(...);
static_assert(sizeof(f<0>((int(*)[0])0)) == sizeof(long),"zero substitution");
template<int N> int body(){int x[N];return sizeof(x);}
int main(){return body<0>();}
