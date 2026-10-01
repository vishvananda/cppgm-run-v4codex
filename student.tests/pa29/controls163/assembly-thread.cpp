extern "C" void update(int* p) { asm volatile("lock; incl %0" : "+m"(*p) :: "memory","cc"); }
extern "C" int concurrent();
int main() {return concurrent();}
