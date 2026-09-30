extern "C" int fesetround(int) noexcept;
void* __builtin_memcpy(void*,const void*,unsigned long);
unsigned long __builtin_strlen(const char*);
int main() {
  int x=0; int* p=&x;
  __builtin_prefetch(p + x++,1,2);
  if(__builtin_assume_aligned(p,x*alignof(int))!=p)return 8;
  if(x!=1 || __builtin_assume_aligned(p,alignof(int),0)!=p)return 1;
  int modes[4]={0,1024,2048,3072};int expected[4]={1,3,2,0};
  for(int i=0;i<4;++i) {if(fesetround(modes[i]))return 2;if(__builtin_flt_rounds()!=expected[i])return 3;}
  fesetround(0);
  char text[4];__builtin_memcpy(text,"abc",4);
  if(__builtin_strlen(text)!=3)return 4;
  int* allocated=(int*)__builtin_operator_new(sizeof(int));
  *allocated=19;int value=*allocated;__builtin_operator_delete(allocated);
  return value!=19;
}
