// Test support only: Clang produces actual captured block records. These tests
// invoke live stack blocks; they do not call the Blocks allocation runtime.
extern "C" { void* _NSConcreteStackBlock[32]; void* _NSConcreteGlobalBlock[32]; }
struct Small { long a; double b; };
struct Large { long a,b,c; };
int call_int(int (^)(int),int);
double call_float(double (^)(float,double),float,double);
Small call_small(Small (^)(Small),Small);
Large call_large(Large (^)(Large),Large);
int& call_reference(int& (^)(int&),int&);
int call_variadic(int (^)(int,...),int);
void call_void(void (^)(int&),int&);
int call_throw(int (^)(int),int);
typedef int (^Callback)(int);
Callback return_block(Callback);
int self_catch(Callback);
void throw_block(Callback);
int catch_block(void (*)(Callback),Callback);
void host_throw(Callback b){throw b;}
int main(){
 int capture=7;
 Callback block=^(int x){return x+capture;};
 if(call_int(block,4)!=11 || return_block(block)(5)!=12)return 1;
 if(call_float(^(float x,double y){return x+y+capture;},1.5,2.5)!=11.0)return 2;
 Small small={3,2.5};
 Small s=call_small(^(Small x){x.a+=capture;x.b+=1;return x;},small);
 if(s.a!=10 || s.b!=3.5)return 3;
 Large large={1,2,3};
 Large l=call_large(^(Large x){x.a+=capture;x.c+=x.b;return x;},large);
 if(l.a!=8 || l.b!=2 || l.c!=5)return 4;
 int x=5;
 int& ref=call_reference(^int&(int& v){return v;},x);ref=8;
 call_void(^(int& v){v+=capture;},x);if(x!=15)return 5;
 if(call_variadic(^(int v,...){__builtin_va_list ap;__builtin_va_start(ap,v);double f=__builtin_va_arg(ap,double);int i=__builtin_va_arg(ap,int);__builtin_va_end(ap);return v+int(f)+i+capture;},1)!=14)return 6;
 if(call_throw(^int(int n) {throw n+capture;},2)!=10)return 7;
 try{throw_block(block);return 8;}catch(Callback& b){if(b(4)!=11)return 9;}
 if(catch_block(host_throw,block)!=10)return 10;
 if(self_catch(block)!=11)return 11;
 return 0;
}
