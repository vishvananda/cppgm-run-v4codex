typedef char Bytes __attribute__((vector_size(8)));
typedef short Shorts __attribute__((vector_size(8)));
typedef int Ints __attribute__((vector_size(8)));
typedef long long Wide __attribute__((vector_size(8)));
int main(int argc,char**) {
    Shorts a={-32768,-129,127,32767},b={-1,0,128,256};
    Bytes s=__builtin_ia32_packsswb(a,b);
    if(s[0]!=-128||s[1]!=-128||s[2]!=127||s[3]!=127||s[4]!=-1||s[5]!=0||s[6]!=127||s[7]!=127)return 1;
    Bytes u=__builtin_ia32_packuswb(a,b);
    if((unsigned char)u[0]!=0||(unsigned char)u[3]!=255||(unsigned char)u[6]!=128||(unsigned char)u[7]!=255)return 2;
    Shorts lo=__builtin_ia32_punpcklwd(a,b),hi=__builtin_ia32_punpckhwd(a,b);
    if(lo[0]!=-32768||lo[1]!=-1||lo[2]!=-129||lo[3]!=0||hi[0]!=127||hi[1]!=128||hi[2]!=32767||hi[3]!=256)return 3;
    Shorts add=__builtin_ia32_paddsw(a,b),sub=__builtin_ia32_psubusw(b,a);
    if(add[0]!=-32768||add[3]!=32767||sub[0]!=32767||sub[1]!=0||sub[2]!=1||sub[3]!=0)return 4;
    Shorts shifted=__builtin_ia32_psrawi(a,argc+15);
    Shorts zero=__builtin_ia32_psrlwi(a,argc+15);
    if(shifted[0]!=-1||shifted[1]!=-1||shifted[2]!=0||shifted[3]!=0||zero[0]!=0||zero[3]!=0)return 5;
    Shorts count=(Shorts)Wide{0x100000000LL};
    zero=__builtin_ia32_psllw(a,count);if(zero[0]!=0||zero[2]!=0)return 6;
    Ints products=__builtin_ia32_pmaddwd(Shorts{-32768,-32768,(short)argc,2},Shorts{-32768,-32768,3,4});
    if((unsigned)products[0]!=2147483648u||products[1]!=11)return 7;
    Shorts upper=__builtin_ia32_pmulhw(Shorts{-32768,32767,-2,2},Shorts{2,2,32767,32767});
    if(upper[0]!=-1||upper[1]!=0||upper[2]!=-1||upper[3]!=0)return 8;
    Ints eq=__builtin_ia32_pcmpeqd(Ints{argc,2},Ints{1,3});
    return eq[0]!=-1||eq[1]!=0;
}
