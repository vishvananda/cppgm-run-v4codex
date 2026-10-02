extern "C" int printf(const char*,...);
unsigned long long hash=1469598103934665603ULL;
template<class T> void consume(T value) {
 unsigned char data[sizeof(T)];__builtin_memcpy(data,&value,sizeof(T));
 for(unsigned i=0;i<sizeof(T);++i) {hash^=data[i];hash*=1099511628211ULL;}
}
typedef char A __attribute__((vector_size(16)));
typedef char B __attribute__((vector_size(16)));
int main(int argc,char**){ A a={};for(unsigned i=0;i<sizeof(a)/sizeof(a[0]);++i)a[i]=(char)((i*32771u)^((unsigned)argc*0x8f01u));B b={};for(unsigned i=0;i<sizeof(b)/sizeof(b[0]);++i)b[i]=(char)((i%2 ? -3 : 5)+argc);consume(__builtin_ia32_punpckhbw128(a,b));printf("%llu\n",hash);return 0;}
