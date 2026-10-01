struct named { int value; };
struct left { typedef int number; using record = named; };
struct right { using number = int; typedef named record; };
struct both : left,right { number value; record obj; };
struct reverse : right,left { number value; record obj; };
template<class T> struct a { typedef T name; };
template<class T> struct b { using name = T; };
template<class T> struct c : a<T>, b<T> { typename c::name value; };
static_assert(sizeof(both::number)==sizeof(int), "merged member types");
static_assert(sizeof(reverse::record)==sizeof(named), "base order");
int main() { both x; x.value=13; x.obj.value=17; c<int> y; y.value=19; return x.value+x.obj.value+y.value==49?0:1; }
