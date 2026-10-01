template<class... T> struct List {};
template<unsigned N> struct Width {typedef char V __attribute__((vector_size(N)));};
template<unsigned... N> struct Shapes { using types=List<typename Width<N>::V...>; };
using Result=Shapes<4,8,16>::types;
using Expected=List<Width<4>::V,Width<8>::V,Width<16>::V>;
static_assert(__is_same(Result,Expected),"vector pack identities");
int main(){return 0;}
