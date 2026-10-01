#include <typeinfo>
using F=_Complex float;
using D=_Complex double;
using L=_Complex long double;
template<class T> int caught(T x) {
  try {throw x;} catch(const T& y) {
    return __real__ y==3&&__imag__ y==4;
  }
}
int main() {
  F f=__builtin_complex(3.0f,4.0f);
  D d=__builtin_complex(3.0,4.0);
  L l=__builtin_complex(3.0L,4.0L);
  if(!caught(f)||!caught(d)||!caught(l))return 1;
  if(typeid(F)==typeid(D)||typeid(D)==typeid(L)||typeid(f)!=typeid(F))return 2;
  try {throw &d;}catch(D* p){if(p!=&d)return 3;}
  try {throw d;}catch(F){return 4;}catch(D x){if(__imag__ x!=4)return 5;}
  return 0;
}
