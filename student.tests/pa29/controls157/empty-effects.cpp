int count;
struct E { E(){++count;} E(const E&){count+=2;} ~E(){count+=4;} };
struct H {[[no_unique_address]] E e; H()=default;};
struct S {int x; [[no_unique_address]] H h; S():x(0x12345678),h(){} };
struct A { int x; [[no_unique_address]] H h; };
int main(){ A arr[2]={{9},{11}}; if(arr[0].x!=9||arr[1].x!=11||count!=2)return 3; count=0; {S s; if(s.x!=0x12345678||count!=1)return 1;} return count!=5; }
