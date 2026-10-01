extern "C" int printf(const char*,...);
int effects;
struct L {int line; constexpr L(int n=__builtin_LINE()):line(n){}};
int ref(const unsigned& n=__builtin_LINE()){return n;}
int value(L l=L()){return l.line;}
int nested(L l=L(__builtin_LINE())){return l.line;}
int counted(int n=(++effects,__builtin_LINE())){return n;}
struct R {int line=__builtin_LINE(); R(){} R(int){}};
R global;
R global2(1);
constexpr const char* file(const char* p=__builtin_FILE()){return p;}
constexpr const char* name(const char* p=__builtin_FUNCTION()){return p;}
#line 500 "values.cpp"
static_assert(file()[0]=='v', "file caller");
static_assert(name()[0]==0, "no caller function");
int main(){
 int a=ref(), b=value(), c=nested(), d=counted();
 int e=ref();
 printf("%d %d %d %d %d %d %d %d\n",a,b,c,d,e,effects,global.line,global2.line);
}
