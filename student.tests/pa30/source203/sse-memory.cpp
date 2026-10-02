typedef char B __attribute__((vector_size(16)));
typedef float F __attribute__((vector_size(16)));
typedef float Pair __attribute__((vector_size(8)));
typedef long long Q __attribute__((vector_size(16)));
int main() {
    Pair pair={8,9};
    F value=__builtin_ia32_loadhps(F{1,2,3,4},&pair);
    if(value[0]!=1||value[1]!=2||value[2]!=8||value[3]!=9)return 1;
    __builtin_ia32_storelps(&pair,value);if(pair[0]!=1||pair[1]!=2)return 2;
    char bytes[16]={};
    B a={1,2,3,4,5,6,7,8},mask={-1,0,-1,0,-1,0,-1,0};
    __builtin_ia32_maskmovdqu(a,mask,bytes);__builtin_ia32_sfence();
    if(bytes[0]!=1||bytes[1]!=0||bytes[2]!=3||bytes[6]!=7||bytes[15]!=0)return 3;
    Q q={1,2}; q=__builtin_ia32_pslldqi128(q,64);if(q[0]!=0||q[1]!=1)return 4;
    q=__builtin_ia32_psrldqi128(q,64);if(q[0]!=1||q[1]!=0)return 5;
    q=__builtin_ia32_pslldqi128(q,128);return q[0]!=0||q[1]!=0;
}
