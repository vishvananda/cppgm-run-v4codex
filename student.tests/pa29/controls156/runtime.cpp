#if !__has_builtin(__builtin_memcpy) || !__has_builtin(__builtin_memset) || !__has_builtin(__builtin_sqrtf) || !__has_builtin(__builtin_frexpl) || !__has_builtin(__builtin_popcountg)
#error registry probe mismatch
#endif
int main(int argc,char**) {
  char text[12]={}; const char* src="scalar";
  if(__builtin_memcpy(text,src,7)!=text || __builtin_strlen(text)!=6)return 1;
  if(__builtin_strchr(text,'a')!=text+2 || __builtin_strrchr(text,'a')!=text+4)return 2;
  __builtin_memmove(text+1,text,6);
  if(__builtin_memcmp(text,"sscalar",7))return 3;
  __builtin_bzero(text,12);
  if(text[1] || text[6])return 4;
  __builtin_memset(text,argc+64,3);
  if(__builtin_memchr(text,'A',12)!=text)return 5;
  float x=argc*4.0f; int exponent=0;
  if(::__builtin_sqrtf(x)!=2.0f || __builtin_ldexp(0.5,3)!=4.0)return 6;
  if(__builtin_frexp(8.0,&exponent)!=0.5 || exponent!=4)return 7;
  double whole=0;
  if(__builtin_modf(3.25,&whole)!=0.25 || whole!=3.0)return 8;
  if(__builtin_lround(2.75)!=3 || __builtin_ilogb(8.0)!=3)return 9;
  return 0;
}
