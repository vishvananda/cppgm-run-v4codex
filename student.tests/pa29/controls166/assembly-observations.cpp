int destroyed;
struct Box { int value; ~Box(){ ++destroyed; } };
Box make(int n) { return Box{n}; }
template<class T> int register_output() {
    T flag = 0;
    asm("movl $1,%0" : "=r"(flag));
    int selected = flag ? make(7).value : make(9).value;
    return selected;
}
template<class T> int memory_output() {
    T flag = 0;
    asm("incl %0" : "+m"(flag));
    int selected = flag ? make(11).value : make(13).value;
    return selected;
}
int main() {
    int flag = 0;
    asm("movl $1,%0" : "=r"((flag)));
    int selected = flag ? make(7).value : make(9).value;
    if (selected != 7) return 1;
    int flag2 = 1;
    asm("xorl %0,%0" : "+r"(flag2));
    int selected2 = flag2 ? make(5).value : make(3).value;
    if (selected2 != 3) return 2;
    int flag3 = 1;
    asm("movl $0,%0" : "=m"(flag3));
    int selected3 = flag3 ? make(5).value : make(3).value;
    if (selected3 != 3) return 3;
    return register_output<int>() == 7 && memory_output<int>() == 11 && destroyed > 0 ? 0 : 4;
}
