int destroyed;
struct Guard { int value; Guard(int n):value(n){} ~Guard(){++destroyed;} };
int throwing(){throw 3;}
void hint() noexcept { asm volatile("pause" ::: "memory"); }
int main(){
  int x=1;
  {Guard guard(8); asm("addl %1,%0" : "+r"(x) : "r"(Guard(4).value)); if(x!=5||destroyed!=1)return 1; hint();}
  if(destroyed!=2)return 2;
  try {Guard guard(7);asm("addl %1,%0" : "+r"(x) : "r"(throwing()));return 3;} catch(int n) {if(n!=3||x!=5||destroyed!=3)return 4;}
  return 0;
}
