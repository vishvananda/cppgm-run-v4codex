using F=_Complex float;
using D=_Complex double;
using L=_Complex long double;
F fm(F x,F y){return x*y;}
F fd(F x,F y){return x/y;}
L lm(L x,L y){return x*y;}
L ld(L x,L y){return x/y;}
D dm(D x,D y){return x*y;}
D dd(D x,D y){return x/y;}
D choose(bool b,F x,D y){return b?x:y;}
int main(int argc,char**){
 F f=__builtin_complex(3.0f,4.0f);
 F f2=fm(f,f),f3=fd(f2,f);
 if(__real__ f2!=-7||__imag__ f2!=24||__real__ f3!=3||__imag__ f3!=4)return 1;
 L l=__builtin_complex(3.0L,4.0L);
 L l2=lm(l,l),l3=ld(l2,l);
 if(__real__ l2!=-7||__imag__ l2!=24||__real__ l3!=3||__imag__ l3!=4)return 2;
 D large=__builtin_complex(1e300,1e300);
 D q=dd(large,large);
 if(__real__ q!=1||__imag__ q!=0)return 3;
 D inf=dm(__builtin_complex(__builtin_inf(),0.0),__builtin_complex(1.0,1.0));
 if(!__builtin_isinf(__real__ inf)||!__builtin_isinf(__imag__ inf))return 4;
 D a=choose(argc!=0,f,large),b=choose(argc==0,f,large);
 if(__real__ a!=3||__imag__ a!=4||__real__ b!=1e300||__imag__ b!=1e300)return 5;
 return 0;
}
