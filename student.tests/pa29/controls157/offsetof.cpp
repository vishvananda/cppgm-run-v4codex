struct E {};
struct Inner { char x; int data[4]; };
struct Object { char first; Inner array[3]; [[no_unique_address]] E empty; };
struct Anonymous { char c; struct { int x; char y; }; };
template<class T,int N> constexpr unsigned long offset() { return __builtin_offsetof(T,array[N].data[2]); }
static_assert(__builtin_offsetof(Object,array[1].data[2])==36,"nested path");
static_assert(offset<Object,2>()==56,"dependent path");
static_assert(__builtin_offsetof(Object,empty)==0,"empty path");
static_assert(__builtin_offsetof(Anonymous,y)==8,"anonymous storage");
int main(int argc,char**){ int i=argc; unsigned long k=__builtin_offsetof(Object,array[i++].data[2]);
 return k!=36 || i!=2; }

static_assert(::__builtin_offsetof(Object,array)==4,"global qualification");
