#include <stdarg.h>
extern "C" int host_consume(int, va_list);
extern "C" int host_call();
extern "C" int consume(int count, ...) {
    va_list ap, copy;
    va_start(ap,count);
    va_copy(copy,ap);
    long result=0;
    for(int i=0;i<count;++i) result+=va_arg(ap,int);
    for(int i=0;i<10;++i) result+=(int)va_arg(ap,double);
    result+=(int)va_arg(ap,long double);
    int *p=va_arg(ap,int*);
    result+=*p;
    int host=host_consume(count,copy);
    va_end(copy);va_end(ap);
    return result==165 && host==165 ? 0:1;
}
template<class T> T next(va_list ap) { return va_arg(ap,T); }
int one(int n,...) {va_list ap;va_start(ap,n);int result=next<int>(ap);va_end(ap);return result;}
int main(){int *p=(int*)__builtin_alloca(17*sizeof(int));p[16]=42;
return p[16]==42 && one(1,7)==7 ? host_call():1;}
