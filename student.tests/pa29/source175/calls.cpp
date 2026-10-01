extern "C" int printf(const char*,...);
constexpr int loc(int n=__builtin_LINE()){return n;}
constexpr int add(int n=loc()+__builtin_LINE()){return n;}
constexpr int fix(){return loc();}
constexpr int twice(){return add();}
int runtime(int n=loc()){return n;}
struct C {
 int n;
 constexpr C(int x=loc()):n(x){}
};
struct F { int operator()(int n=loc()) const {return n;} };
struct M { int n=loc(); M(){} M(int){} };
#line 200 "caller.cpp"
static_assert(loc()==200, "first");
static_assert(loc()==201, "second");
static_assert(add()==404, "nested mixed");
static_assert(fix()==4, "lexical callee");
static_assert(twice()==10, "nested lexical callee");
static_assert(C().n==205, "constructor default");
static_assert(C{10}.n==10, "explicit constructor");
static_assert(noexcept(::__builtin_LINE()), "no unwind");
static_assert(sizeof(decltype(__builtin_FILE()))==8, "query type");
int main(){
 C c;
 C d{};
 F f;
 printf("%d %d %d %d %d %d %d %d\n", runtime(), loc(9), add(),fix(),c.n,d.n,f(),M().n);
}
