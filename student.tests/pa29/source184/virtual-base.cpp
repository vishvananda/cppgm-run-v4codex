struct B {int x; B():x(7){}};
struct P {int p; P():p(3){}};
struct D:P,virtual B {};
int main(){D d; const D *p=&d; B *b=(B*)p; B& r=(B&)(const D&)d;
 b->x=11; r.x+=2;return d.x==13 && d.p==3?0:1;}
