typedef int Ints __attribute__((vector_size(8)));
typedef short Shorts __attribute__((vector_size(8)));
unsigned long long input(unsigned x) { return (static_cast<unsigned long long>(x)<<32) | 0x23456789u; }
int main(int argc,char**) {
    unsigned long long n=input(argc+17);
    Ints v=(Ints)n;
    Shorts s=(Shorts)v;
    unsigned long long back=(unsigned long long)s;
    return __builtin_ia32_vec_ext_v2si(v,0)!=0x23456789 || __builtin_ia32_vec_ext_v2si(v,1)!=argc+17 || back!=n || __builtin_ia32_vec_ext_v4hi(s,0)!=0x6789;
}
