extern "C" int printf(const char*,...);
unsigned long long hash=1469598103934665603ULL;
template<class T> void consume(T value) {
 unsigned char data[sizeof(T)];__builtin_memcpy(data,&value,sizeof(T));
 for(unsigned i=0;i<sizeof(T);++i) {hash^=data[i];hash*=1099511628211ULL;}
}
typedef long long A __attribute__((vector_size(8)));
typedef int B;
int main(int argc,char**){ A a={};for(unsigned i=0;i<sizeof(a)/sizeof(a[0]);++i)a[i]=(long long)((i*32771u)^((unsigned)argc*0x8f01u));B b={};b=argc+1;consume(__builtin_ia32_psllqi(a,0));consume(__builtin_ia32_psllqi(a,8));consume(__builtin_ia32_psllqi(a,64));consume(__builtin_ia32_psllqi(a,128));consume(__builtin_ia32_psllqi(a,255));printf("%llu\n",hash);return 0;}
