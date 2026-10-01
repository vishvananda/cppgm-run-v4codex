struct Zero { int data[0]; };
struct Header { char tag; long data[0]; };
int seen;
Zero pass(Zero, int value) {seen=value;return {};}
long field(Header h,long salt) {return h.tag+salt;}
int pick(int (*)[0]) {return 11;}
int pick(int (*)[]) {return 22;}
