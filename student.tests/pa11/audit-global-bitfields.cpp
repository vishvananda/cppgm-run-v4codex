// Static packing must exist before any dynamic initializer reads the object.
struct A { char lead; unsigned x:3; unsigned y:5; char tail; signed z:7; };
extern A a;
int check() { return a.lead!=42 || a.x!=3 || a.y!=17 || a.tail!=23 || a.z!=-9; }
int observed=check();
A a={42,3,17,23,-9};
struct Wide { unsigned long x:41; unsigned long y:23; };
Wide w={1099511627779ul,1234567ul};
struct Small { unsigned char x:3; unsigned char y:2; bool b:1; };
Small s[2]={{5,2,true},{7,1,false}};
int main(){return observed || check() || w.x!=1099511627779ul || w.y!=1234567ul || s[0].x!=5 || s[0].y!=2 || !s[0].b || s[1].x!=7 || s[1].y!=1 || s[1].b;}
