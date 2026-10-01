using C=char __attribute__((ext_vector_type(1)));
using B=bool __attribute__((ext_vector_type(4)));
using S=short __attribute__((ext_vector_type(4)));
using I=int __attribute__((ext_vector_type(4)));
extern "C" { C nc(C); B nb(B); S ns(S); I ni(I); int native_test(); C hc(C); B hb(B); S hs(S); I hi(I); }
using W=int __attribute__((ext_vector_type(8)));
using X=int __attribute__((ext_vector_type(16)));
using Y=int __attribute__((ext_vector_type(32)));
extern "C" { W nw(W); X nx(X); Y ny(Y); W hw(W); X hx(X); Y hy(Y); int native_wide(); }
