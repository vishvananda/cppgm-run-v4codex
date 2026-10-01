template<class T,T*... I> struct pointer_seq {};
template<class T,T... I> struct value_seq {};
template<template<class,int...> class S> struct fixed_values {};
template<template<class,int*...> class S> struct fixed_pointers {};
#ifdef POINTER_TO_VALUE
fixed_values<pointer_seq> a;
#elif defined(VALUE_TO_POINTER)
fixed_pointers<value_seq> a;
#else
fixed_pointers<pointer_seq> a;
#endif
int main(){return 0;}
