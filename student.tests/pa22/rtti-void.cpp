struct B { virtual int f(){return 1;} };
struct D:B {int x;int f(){return 2;}};
int main(){D d;B*p=&d;const B*cp=&d;
 if(dynamic_cast<void*>(p)!=&d || dynamic_cast<const void*>(cp)!=&d)return 1;
 p=0;return dynamic_cast<void*>(p)!=0;}
