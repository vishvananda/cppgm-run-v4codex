using F=_Complex float; using D=_Complex double; using L=_Complex long double;
F f(F x){return x;} D d(D x){return x;} L l(L x){return x;}
D global=__builtin_complex(3.0,4.0);
int main(){
 F a=f(__builtin_complex(1.0f,-2.0f)); D b=d(global); L c=l(__builtin_complex(5.0L,-6.0L));
 return __real__ a==1 && __imag__ a==-2 && __real__ b==3 && __imag__ b==4 && __real__ c==5 && __imag__ c==-6 ? 0:1;
}
