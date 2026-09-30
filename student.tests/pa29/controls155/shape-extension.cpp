struct Plain {int n;};
struct Constructor {Constructor(){};};
struct WithVirtual {virtual void f();};
static_assert(__is_aggregate(Plain) && __is_aggregate(int[3]), "aggregate");
static_assert(!__is_aggregate(Constructor) && !__is_aggregate(WithVirtual), "not aggregate");
static_assert(!__is_aggregate(int) && !__is_aggregate(Plain&), "nonaggregate shapes");
static_assert(__array_rank(int[2][3][4]) == 3, "rank");
static_assert(__array_rank(int[][4]) == 2, "incomplete outer rank");
static_assert(__array_rank(int(&)[3]) == 0, "reference rank");
template<class T> struct Rank {static const decltype(__array_rank(T)) value = __array_rank(T);};
static_assert(Rank<int[2][4]>::value == 2, "dependent rank");
static_assert(__is_same(__remove_reference(int&&),int), "transform alias");
static_assert(__is_convertible_to(int,double), "convertible alias");
namespace N { using ::Plain __attribute__((using_if_exists)); using ::Absent __attribute__((using_if_exists)); }
using Pointer [[gnu::nodebug]] = __add_pointer(int&);
static_assert(__is_same(Pointer,int*), "attributed alias");
int main(){N::Plain p; p.n=0; return p.n;}

namespace ordinary_names {
template<class T> using __decay_t = T;
template<class T> struct __remove_reference { typedef T type; };
static_assert(__is_same(__decay_t<int>,int),"ordinary alias");
static_assert(__is_same(__remove_reference<int>::type,int),"ordinary class");
}
