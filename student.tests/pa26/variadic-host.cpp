#include <cstdarg>
extern "C" int consume(int, ...);
extern "C" int host_consume(int count, va_list ap) {
    long result=0;
    for(int i=0;i<count;++i)result+=va_arg(ap,int);
    for(int i=0;i<10;++i)result+=(int)va_arg(ap,double);
    result+=(int)va_arg(ap,long double);
    result+=*va_arg(ap,int*);
    return result;
}
extern "C" int host_call(){int n=17;return consume(12,1,2,3,4,5,6,7,8,9,10,11,12,
    1.,2.,3.,4.,5.,6.,7.,8.,9.,10.,15.L,&n);}
