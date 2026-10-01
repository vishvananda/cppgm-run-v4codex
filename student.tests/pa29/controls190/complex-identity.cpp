using F=_Complex float;
using D=_Complex double;
using L=_Complex long double;
constexpr F f1=__builtin_complex(3.0f,4.0f),f2=__builtin_complex(3.0f,9.0f);
constexpr D d1=__builtin_complex(3.0,4.0),d2=__builtin_complex(3.0,9.0);
constexpr L l1=__builtin_complex(3.0L,4.0L),l2=__builtin_complex(3.0L,9.0L);
constexpr D plus=__builtin_complex(3.0,0.0),minus=__builtin_complex(3.0,-0.0);
template<int N> struct values {static constexpr D value=__builtin_complex(3.0,double(N));};
template<int N> constexpr D values<N>::value;
double read(const D& c){return __imag__ c;}
int main(){
  if(__imag__ f1!=4||__imag__ f2!=9)return 1;
  if(read(d1)!=4||read(d2)!=9)return 2;
  if(__imag__ l1!=4||__imag__ l2!=9)return 3;
  if(read(values<4>::value)!=4||read(values<9>::value)!=9)return 4;
  if(__builtin_signbit(__imag__ plus)||!__builtin_signbit(__imag__ minus))return 5;
  return 0;
}
