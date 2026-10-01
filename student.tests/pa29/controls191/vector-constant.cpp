using I=int __attribute__((ext_vector_type(4)));
using B=bool __attribute__((ext_vector_type(4)));
constexpr I a{1,2,3,4};
static_assert(__builtin_reduce_or(a)==7,"reduction");
static_assert(__builtin_reduce_or(a==a)==-1,"comparison mask");
constexpr B mask=__builtin_convertvector(a,B);
static_assert(__builtin_reduce_or(mask),"packed boolean lanes");
constexpr int f(I v){return __builtin_reduce_or(v);}
static_assert(f(a)==7,"argument value");
int main(){return __builtin_reduce_or(mask)?0:1;}
