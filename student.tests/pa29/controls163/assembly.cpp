int evaluations;
int& locate(int& x) { ++evaluations; return x; }
int input(int x) { ++evaluations; return x; }
template<class T> T swap_bytes(T x) { asm volatile("bswap %0" : "+r"(x)); return x; }
template<class T> void increment(T& x) { __asm__ __volatile__("lock; incl %[object]" : [object] "+m"(x) :: "cc","memory"); }
int main() {
    asm("nop"); asm volatile("pause"); asm volatile("rep; nop" ::: "memory");
    asm volatile("mfence"); asm volatile("" ::: "memory");
    if (swap_bytes(0x12345678u) != 0x78563412u) return 1;
    if (swap_bytes(0x0123456789abcdefUL) != 0xefcdab8967452301UL) return 2;
    int x=7;
    asm("addl %1,%0; negl %0; notl %0" : "+r"(locate(x)) : "r"(input(4)) : "cc");
    if(x!=10 || evaluations!=2) return 3;
    int copied=0; asm("movl %1,%0; subl $3,%0" : "=&r"(copied) : "r"(x));
    if(copied!=7) return 4;
    asm("incl %0" : "=r"(copied) : "0"(x)); if(copied!=11) return 5;
    increment(x); if(x!=11) return 6;
    unsigned char byte=0x55; asm("lock; notb %0" : "+m"(byte) :: "memory"); if(byte!=0xaa) return 7;
    short small=123; asm("lock decw %0" : "+m"(small) :: "cc"); if(small!=122) return 8;
    long wide=100; asm("lock; addq %1,%0; lock; xorq $15,%0" : "+m"(wide) : "r"(17L) : "cc","memory"); if(wide!=(117L^15L)) return 9;
    int y=21; asm("xchgl %1,%0" : "+m"(x), "+r"(y) :: "memory"); if(x!=21||y!=11) return 10;
    y=5; asm("lock xaddl %1,%0" : "+m"(x), "+r"(y) :: "memory","cc"); if(x!=26||y!=21) return 11;
    asm("andl $15,%0; orl $32,%0; xorl $2,%0" : "+r"(x)); if(x!=40) return 12;
    int alias=5; asm("movl $12,%0; addl %1,%0" : "+r"(alias) : "r"(alias)); if(alias!=17) return 13;
    int memory=3, value=0; asm("movl %1,%0" : "=r"(value) : "m"(memory)); if(value!=3) return 14;
    asm("movl %1,%0" : "=m"(memory) : "r"(value)); if(memory!=3) return 15;
    asm("incl %[result]" : [result] "=r"(copied) : "[result]"(value)); if(copied!=4)return 16;
    asm("xaddl %0,%0" : "+r"(copied)); if(copied!=8)return 17;
    asm("xchgl %0,%1" : "+m"(memory), "+r"(copied)); if(memory!=8||copied!=3)return 18;
    unsigned v=0;asm("movl %1,%0":"=r"(v):"i"(0xabcdefff));if(v!=0xabcdefffu)return 19;
    return 0;
}
