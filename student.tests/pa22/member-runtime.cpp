struct C { int bias; int f(int x)const{return bias+x;} };
int main(){volatile int count=4000000;volatile int input=3;C c;c.bias=input;
 int(C::*p)(int)const=&C::f;unsigned long sum=0;
 for(int i=0;i<count;++i)sum+=(c.*p)(i&7);
 return sum!=26000000;}
