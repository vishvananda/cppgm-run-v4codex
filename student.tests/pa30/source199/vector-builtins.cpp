typedef signed char Bytes __attribute__((vector_size(8)));
typedef short Shorts __attribute__((vector_size(8)));
typedef int Ints __attribute__((vector_size(8)));
int calls;
int next(int v) { ++calls; return v; }
template<int N> int extract(Ints v) { return __builtin_ia32_vec_ext_v2si(v,N); }
static_assert(__is_same(decltype(__builtin_ia32_vec_ext_v4hi(Shorts{},0)),short), "lane type");
static_assert(sizeof(decltype(__builtin_ia32_vec_init_v4hi(1,2,3,4)))==8, "width");
static_assert(noexcept(__builtin_ia32_vec_init_v2si(1,2)), "nothrow");
int main(int argc, char**) {
    Bytes b = __builtin_ia32_vec_init_v8qi(next(argc),2,3,4,5,6,7,-8);
    Shorts s = __builtin_ia32_vec_init_v4hi(32767,-2,3,next(4));
    Ints i = __builtin_ia32_vec_init_v2si(next(37),next(argc+8));
    signed char bytes[8]; __builtin_memcpy(bytes,&b,8);
    return bytes[0]!=argc || bytes[7]!=-8 || __builtin_ia32_vec_ext_v4hi(s,0)!=32767 || __builtin_ia32_vec_ext_v4hi(s,1)!=-2 || __builtin_ia32_vec_ext_v4hi(s,3)!=4 ||
        __builtin_ia32_vec_ext_v4hi(s,0)!=32767 ||
        extract<1>(i)!=argc+8 || extract<0>(i)!=37 || calls!=4;
}
