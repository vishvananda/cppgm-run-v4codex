struct Header { char tag; long data[0]; };
struct Interior { char tag; int data[0]; char tail; };
struct Empty { int data[0]; };
union Union { char tag; long data[0]; };
static_assert(sizeof(Header) == 8 && alignof(Header) == 8, "tail padding");
static_assert(__builtin_offsetof(Header,data) == 8, "tail offset");
static_assert(sizeof(Interior) == 8 && __builtin_offsetof(Interior,tail) == 4, "interior alignment");
static_assert(sizeof(Empty) == 0 && alignof(Empty) == 4, "zero-only class");
static_assert(sizeof(Union) == 8 && __builtin_offsetof(Union,data) == 0, "union alignment");
int main() { Header h = {7,{}}; Interior i = {2,{},9}; return h.tag != 7 || i.tail != 9; }
