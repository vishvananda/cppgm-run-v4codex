struct Zero { int data[0]; };
struct Header { char tag; long data[0]; };
extern int seen;
Zero pass(Zero,int); long field(Header,long);int pick(int(*)[0]);int pick(int(*)[]);
int main(){Zero z{};Zero y=pass(z,37);Header h{9,{}};return sizeof(y)!=0||seen!=37||field(h,5)!=14||pick((int(*)[0])0)!=11||pick((int(*)[])0)!=22;}
