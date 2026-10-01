extern "C" int printf(const char*,...);
constexpr int loc(int n=__builtin_LINE()){return n;}
struct Plain {
 int n=__builtin_LINE();
 const char* f=__builtin_FUNCTION();
 Plain(){}
 Plain(int){}
};
struct Auto {int n=loc();};
struct Base {int n; Base(int x=loc()):n(x){}};
struct Derived:Base {Derived(){}};
struct Explicit:Base {Explicit():Base(loc()){}};
template<class T> struct Box {
 int n=loc(); const char* f=__builtin_FUNCTION();
 Box(){}
 Box(T){}
};
#line 300 "objects.cpp"
Base global;
int main(){
 static Base stat;
 Base local;
 Base array[2];
 Base* ptr=new Base;
 printf("%d %d %d %s %d %d %d %d %d %d %d %d %s\n",global.n,stat.n,local.n,
   Plain().f,Plain().n,Plain(1).n,Auto().n,Derived().n,Explicit().n,
   array[1].n,ptr->n,Box<int>().n,Box<long>().f);
 delete ptr;
}
