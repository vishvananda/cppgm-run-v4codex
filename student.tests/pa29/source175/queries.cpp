extern "C" int printf(const char*,...);
constexpr int loc(int x=__builtin_LINE()){return x;}
constexpr const char* name(const char* p=__builtin_FUNCTION()){return p;}
template<int N> struct Number{static const int value=N;};
template<class T> int use(){
 static_assert(__builtin_LINE()==6, "template lexical");
 static_assert(loc()==7, "template default");
 return loc();
}
template<class T> const char* tname(){return __builtin_FUNCTION();}
#line 401 "queries.cpp"
static_assert(Number<loc()>::value==401, "query key first");
static_assert(Number<loc()>::value==402, "query key second");
static_assert(__builtin_FUNCTION()[0]=='\0', "global function empty");
static_assert(__builtin_FILE()[0]=='q', "file string constant");
static_assert(noexcept(__builtin_FUNCTION()), "no unwind");
int main(){
 auto lambda=[](){return name();};
 printf("%d %d %c %s %u\n",use<int>(),use<long>(),tname<int>()[0],lambda(),::__builtin_LINE());
}
