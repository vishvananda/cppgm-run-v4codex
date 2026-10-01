int alive;
struct Empty { Empty(){++alive;} ~Empty(){--alive;} };
template<int N> struct Object {
 typedef int Row[4] __attribute__((aligned(N)));
 char first;
 [[no_unique_address]] Empty empty;
 Row rows[3];
};
static_assert(__builtin_offsetof(Object<16>,rows)==16,"aligned rows");
static_assert(__builtin_offsetof(Object<16>,empty)==0,"overlapping empty");
unsigned long offset(int i){return __builtin_offsetof(Object<16>,rows[i][2]);}
int main(){int result; {Object<16> object={}; object.first=7;
 result=object.first!=7 || alive!=1 || offset(alive)!=40;}
 return result || alive!=0;}
