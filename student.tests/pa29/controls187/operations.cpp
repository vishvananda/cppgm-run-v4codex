using C=_Complex double;
int calls;
double tick(){++calls;return 4;}
struct S {C x;};
C changed(C x){__real__ x+=2;__imag__ x-=1;return x;}
int main(){
 C a=__builtin_complex(-0.0,__builtin_inf());
 if(!__builtin_signbit(__real__ a)||!__builtin_isinf(__imag__ a))return 1;
 C b=__builtin_complex(__builtin_nan(""),-0.0);
 if(!__builtin_isnan(__real__ b)||!__builtin_signbit(__imag__ b))return 2;
 if(__imag__ tick()!=0||calls!=1)return 3;
 double x=3; __real__ x=7; if(x!=7)return 4;
 C c=__builtin_complex(3.0,4.0); C d=-c; C e=~c;
 if(__real__ d!=-3||__imag__ d!=-4||__real__ e!=3||__imag__ e!=-4)return 5;
 if(c!=c || c==d || !c || !bool(c))return 6;
 if(static_cast<double>(c)!=3 || int(c)!=3)return 7;
 c+=2; c*=__builtin_complex(0.0,1.0); c/=__builtin_complex(0.0,1.0);
 if(__real__ c!=5 || __imag__ c!=4)return 8;
 ++c; c--; if(__real__ c!=5||__imag__ c!=4)return 9;
 volatile C observed=changed(c); C read=observed;
 if(__real__ read!=7||__imag__ read!=3)return 10;
 S source={c}; S copy=source; copy=source;
 if(__imag__ copy.x!=4)return 11;
 C* p=&copy.x; double* part=&__imag__ *p; *part=9;
 if(__imag__ copy.x!=9)return 12;
 return 0;
}
