// Optimization coverage: fold the constant helper, retain observable argument
// evaluation and side effects, and keep address-taken definitions callable.
unsigned calls;
volatile int observed;
inline bool checked_mode() { return false; }
inline int get(int value,int divisor) { if(checked_mode()) return value/divisor; return value+3; }
__attribute__((noinline)) int keep() { return 7; }
inline int effect(int x) { observed += x; return 0; }
__attribute__((noinline)) int argument() { ++calls; return 11; }
inline int constant(int) { return 5; }
bool (*volatile addressed)() = &checked_mode;
int main() {
    int sum=0;
    for(int i=0;i<20;++i) {
        sum+=get(i,0);
        if(constant(argument())!=5 || effect(2)!=0 || keep()!=7) return 1;
        if(addressed()) return 2;
    }
    if(sum!=250 || calls!=20 || observed!=40) return 3;
    return 0;
}
