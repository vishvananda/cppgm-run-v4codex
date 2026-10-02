typedef long long Q __attribute__((vector_size(16)));
Q f(Q q){return __builtin_ia32_pslldqi128(q,255);}
