typedef float F4 __attribute__((vector_size(4*sizeof(float))));
using F4Again = float __attribute__((__vector_size__(16)));
typedef int I4 __attribute__((vector_size(16)));
static_assert(sizeof(F4)==16 && alignof(F4)==16,"layout");
static_assert(__is_same(F4,F4Again),"canonical alias");
static_assert(!__is_same(F4,float) && !__is_same(F4,I4),"lane identity");
static_assert(__is_const(const F4) && !__is_array(F4),"shape");
static_assert(!__is_arithmetic(F4) && __is_object(F4),"vector is not scalar");
struct Frame { char prefix; F4 values; char suffix; };
static_assert(sizeof(Frame)==48 && alignof(Frame)==16,"member layout");
static_assert(__builtin_offsetof(Frame,values)==16 && __builtin_offsetof(Frame,suffix)==32,"offset");
typedef F4 LowAlign __attribute__((aligned(1)));
static_assert(sizeof(LowAlign)==16 && alignof(LowAlign)==1,"storage vs shape");
static_assert(__is_same(F4,LowAlign),"storage not identity");
inline F4 full(){return (F4){1,2,3,4};}
inline F4 partial(){return F4{1,2};}
inline void leading(){(void)((__attribute__((vector_size(16))) int){1,2,3,4});}
int main(int argc,char**){return argc==1 && sizeof(Frame)==48?0:1;}
