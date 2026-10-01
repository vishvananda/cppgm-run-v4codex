extern "C" int printf(const char*,...);
constexpr const char* file(const char* p=__builtin_FILE()){return p;}
const char* first=file();
int count=file()[0];
const char* second=__builtin_FILE();
const char* global_name=__builtin_FUNCTION();
struct Two {const char* a; int n; const char* b;};
Two pointers={file(),17,file()};
#line 800 "string-use.cpp"
int main(){
 const char* local=file();
 const char* function=__builtin_FUNCTION();
 printf("%c %c %c %d %d %c %c %c %s\n",first[0],second[0],char(count),global_name[0],
        pointers.n,pointers.a[0],pointers.b[0],local[0],function);
}
