static_assert(__builtin_clz((unsigned char)1)==31,"fixed promotion");
static_assert(__builtin_popcount((signed char)-1)==32,"signed conversion");
static_assert(__builtin_clzg((unsigned char)1)==7,"generic width");
static_assert(__builtin_ctzg((unsigned char)0,91)==91,"fallback");
static_assert(__builtin_popcountg((unsigned __int128)-1)==128,"wide count");
static_assert(__builtin_bswap16(0x1234)==0x3412,"swap");
constexpr int count(unsigned long long x) {return __builtin_popcountll(x);}
static_assert(count(0x8000000000000001ULL)==2,"constexpr activation");
template<class T> int leading(T x) { return __builtin_clzg(x,sizeof(T)*8); }
int counts(unsigned x) {
  int count=0;for(unsigned v=x;v;v>>=1) count+=v&1;
  return count;
}
int main(int argc,char**) {
  for(unsigned x=argc-1;x<65536;++x) {
    int c=counts(x), trailing=0, leading=32;
    for(unsigned v=x;v;v>>=1)--leading;
    if(x) {unsigned v=x;while(!(v&1)){++trailing;v>>=1;}}
    if(__builtin_popcount(x)!=c || __builtin_parity(x)!=(c&1))return 1;
    if(__builtin_ffs(x)!=(x?trailing+1:0))return 2;
    if(x && (__builtin_ctz(x)!=trailing || __builtin_clz(x)!=leading))return 3;
    if(__builtin_ctzg(x,41)!=(x?trailing:41))return 4;
    if(__builtin_clzg(x,-7)!=(x?leading:-7))return 5;
  }
  for(int bit=0;bit<64;++bit) {
    unsigned long long v=1ULL<<bit;
    if(__builtin_ctzll(v)!=bit || __builtin_clzll(v)!=63-bit || __builtin_popcountll(v)!=1)return 6;
  }
  for(int bit=0;bit<128;++bit) {
    unsigned __int128 v=(unsigned __int128)1<<bit;
    if(__builtin_ctzg(v)!=bit || __builtin_clzg(v)!=127-bit || __builtin_popcountg(v)!=1)return 7;
  }
  volatile unsigned char byte=8;
  if(leading(byte)!=4 || __builtin_ctzg(byte)!=3)return 8;
  int effects=0; unsigned value=0;
  if(__builtin_ctzg(value++,++effects)!=1 || effects!=1 || value!=1)return 9;
  volatile unsigned short s=0x1234;
  volatile unsigned long long l=0x0123456789abcdefULL;
  if(__builtin_bswap16(s)!=0x3412 || __builtin_bswap64(l)!=0xefcdab8967452301ULL)return 10;
  return 0;
}
