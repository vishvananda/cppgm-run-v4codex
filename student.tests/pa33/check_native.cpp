#include <sys/mman.h>
#include <cstring>
#include <cstdlib>
#include <initializer_list>
extern "C" long cycle(long,long,long,long,long,long);
extern "C" long prefix(const char*);
extern "C" void copy_unused(const char*,char*,long);
extern "C" void* copy_used(const char*,char*,long);
extern "C" long cppgm_builtin_strlen(const char* p) { return std::strlen(p); }
extern "C" void* cppgm_builtin_memcpy(void* d,const void* s,long n) { return std::memcpy(d,s,n); }
int main() {
    for (long n=0;n!=1000;++n) {
        long a=1,b=2,c=3;
        for (long i=0;i<n;++i) { long old=a;a=b;b=c;c=old; }
        if (cycle(n,1,2,3,4,5)!=a*100+b*10+c+9) return 1;
    }
    char* map=static_cast<char*>(mmap(0,8192,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0));
    if (map==MAP_FAILED || mprotect(map+4096,4096,PROT_NONE)) return 2;
    for (int length=0;length!=128;++length) {
        char* start=map+4095-length;
        std::memset(start,'a',length);start[length]=0;
        if (prefix(start)!=length) return 3;
    }
    for (int offset=0;offset!=32;++offset) for (int length=0;length!=128;++length) {
        std::memset(map+offset,'x',length);map[offset+length]=0;
        if (prefix(map+offset)!=length) return 4;
    }
    char source[4160],dest[4160];
    for (int i=0;i!=4160;++i) source[i]=char(i*23);
    for (int n : {0,1,4,15,16,17,31,64,255,4096}) for (int offset=0;offset!=16;++offset) {
        std::memset(dest,0x7f,sizeof dest);
        copy_unused(source+offset,dest+offset,n);
        if (std::memcmp(dest+offset,source+offset,n) || dest[offset+n]!=0x7f) return 5;
        if (copy_used(source+offset,dest+offset,n)!=dest+offset) return 6;
    }
    munmap(map,8192);
}
