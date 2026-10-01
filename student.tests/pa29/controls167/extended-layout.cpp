typedef int I3 __attribute__((ext_vector_type(3)));
typedef bool B9 __attribute__((ext_vector_type(9)));
typedef bool B2 __attribute__((ext_vector_type(2)));
static_assert(sizeof(I3)==16 && alignof(I3)==16,"padded lanes");
static_assert(sizeof(B9)==2 && alignof(B9)==2 && sizeof(B2)==1,"packed boolean lanes");
static_assert(!__is_same(B2,bool) && __is_literal_type(B2),"mask type");
template<class T, unsigned N> using Vec __attribute__((ext_vector_type(N))) = T;
template<class T> struct Width { static constexpr unsigned value=0; };
template<class T, unsigned N> struct Width<Vec<T,N>> { static constexpr unsigned value=N; };
static_assert(Width<Vec<int,3>>::value==3,"dependent lane deduction");
static_assert(sizeof(Vec<double,3>)==32,"dependent padded layout");
static_assert(sizeof(Vec<bool,17>)==4,"dependent boolean layout");
inline I3 unused(){return I3{1,2,3};}
inline B9 mask(){return B9{true,false,true};}
int check(const I3*){return 3;}
int main(){return check((I3*)0)==3?0:1;}
