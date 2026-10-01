#include <typeinfo>
#include "abi.h"
extern "C" void native_f(F x){throw x;}
extern "C" void native_d(D x){throw x;}
extern "C" void native_l(L x){throw x;}
extern "C" const void* native_type(){return &typeid(D);}
extern "C" const void* native_pointer_type(){return &typeid(D*);}
extern "C" int native_catches(){
  F f=__builtin_complex(3.0f,4.0f);
  D d=__builtin_complex(5.0,6.0);
  L l=__builtin_complex(7.0L,8.0L);
  try{host_f(f);return 1;}catch(const F& v){if(__real__ v!=3||__imag__ v!=4)return 2;}
  try{host_d(d);return 3;}catch(D v){if(__real__ v!=5||__imag__ v!=6)return 4;}
  try{host_l(l);return 5;}catch(const L& v){if(__real__ v!=7||__imag__ v!=8)return 6;}
  return 0;
}
