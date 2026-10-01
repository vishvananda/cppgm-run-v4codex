extern "C" double sum_d(int,...);extern "C" double sum_f(int,...);extern "C" long double sum_l(int,...);extern "C" int own_variadic();
int main(){
 _Complex float f;__real__ f=1;__imag__ f=-2;
 _Complex double d;__real__ d=3;__imag__ d=-4;
 _Complex long double l;__real__ l=5;__imag__ l=-6;
 if(sum_f(9,f,f,f,f,f,f,f,f,f)!=27)return 1;
 if(sum_d(6,d,d,d,d,d,d)!=42)return 2;
 if(sum_l(3,l,l,l)!=33)return 3;
 return own_variadic()?4:0;
}
