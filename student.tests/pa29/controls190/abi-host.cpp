#include <typeinfo>
#include "abi.h"
extern "C" void host_f(F x){throw x;}
extern "C" void host_d(D x){throw x;}
extern "C" void host_l(L x){throw x;}
int main(){
  F f; __real__ f=3; __imag__ f=4;
  D d; __real__ d=5; __imag__ d=6;
  L l; __real__ l=7; __imag__ l=8;
  if(native_catches())return 1;
  if(native_type()!=&typeid(D)||native_pointer_type()!=&typeid(D*))return 2;
  try{native_f(f);return 3;}catch(F x){if(__real__ x!=3||__imag__ x!=4)return 4;}
  try{native_d(d);return 5;}catch(const D& x){if(__real__ x!=5||__imag__ x!=6)return 6;}
  try{native_l(l);return 7;}catch(L x){if(__real__ x!=7||__imag__ x!=8)return 8;}
  return 0;
}
