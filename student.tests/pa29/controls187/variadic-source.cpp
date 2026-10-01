using F=_Complex float; using D=_Complex double; using L=_Complex long double;
extern "C" double sum_d(int n,...) {
 __builtin_va_list ap; __builtin_va_start(ap,n);double sum=0;
 for(int i=0;i<n;++i){D x=__builtin_va_arg(ap,D);sum+=__real__ x-__imag__ x;}
 __builtin_va_end(ap);return sum;
}
extern "C" double sum_f(int n,...) {
 __builtin_va_list ap; __builtin_va_start(ap,n);double sum=0;
 for(int i=0;i<n;++i){F x=__builtin_va_arg(ap,F);sum+=__real__ x-__imag__ x;}
 __builtin_va_end(ap);return sum;
}
extern "C" long double sum_l(int n,...) {
 __builtin_va_list ap; __builtin_va_start(ap,n);long double sum=0;
 for(int i=0;i<n;++i){L x=__builtin_va_arg(ap,L);sum+=__real__ x-__imag__ x;}
 __builtin_va_end(ap);return sum;
}
extern "C" int own_variadic(){
 F f=__builtin_complex(1.0f,-2.0f);D d=__builtin_complex(3.0,-4.0);L l=__builtin_complex(5.0L,-6.0L);
 return sum_f(9,f,f,f,f,f,f,f,f,f)==27 && sum_d(6,d,d,d,d,d,d)==42 && sum_l(3,l,l,l)==33 ? 0:1;
}
