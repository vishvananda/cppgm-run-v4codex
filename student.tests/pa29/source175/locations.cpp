extern "C" int printf(const char*,...);
constexpr int line(int n=__builtin_LINE()){return n;}
constexpr int nested(int n=line()){return n;}
constexpr const char* function(const char* n=__builtin_FUNCTION()){return n;}
constexpr const char* file(const char* n=__builtin_FILE()){return n;}
constexpr int lexical(){return __builtin_LINE();}
template<class T> constexpr int templ(int n=__builtin_LINE()){return n+sizeof(T)-sizeof(T);}
struct S {
  int n=__builtin_LINE();
  const char* f=__builtin_FUNCTION();
  constexpr S(){}
  constexpr S(int){}
};
#define HERE __builtin_LINE()
#line 101 "virtual-source.cpp"
static_assert(HERE==101, "macro line");
static_assert(line()==102, "default line");
static_assert(nested()==103, "nested default line");
static_assert(templ<int>()==104, "template default line");
static_assert(lexical()==6, "body keeps lexical line");
static_assert(S().n==11, "constructor initializer site");
static_assert(S(1).n==12, "other constructor site");
int main(){
 printf("%u %d %d %d %d %s %s %s %s\n",__builtin_LINE(),line(),nested(),templ<long>(),S().n,
   __builtin_FUNCTION(),function(),file(),S().f);
}
