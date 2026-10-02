extern "C" int printf(const char*,...);
unsigned long long hash=1469598103934665603ULL;
template<class T> void consume(T value) {
 unsigned char data[sizeof(T)];__builtin_memcpy(data,&value,sizeof(T));
 for(unsigned i=0;i<sizeof(T);++i) {hash^=data[i];hash*=1099511628211ULL;}
}
typedef short A __attribute__((vector_size(16)));
typedef int B;
int main(int argc,char**){ A a={};for(unsigned i=0;i<sizeof(a)/sizeof(a[0]);++i)a[i]=(short)((i*32771u)^((unsigned)argc*0x8f01u));B b={};b=argc+1;consume(__builtin_ia32_vec_set_v8hi(a,argc,0));consume(__builtin_ia32_vec_set_v8hi(a,argc,1));consume(__builtin_ia32_vec_set_v8hi(a,argc,3));printf("%llu\n",hash);return 0;}
