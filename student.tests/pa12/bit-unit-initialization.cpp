struct Bits {
  unsigned first:1;
  unsigned second:2;
  unsigned :0;
  unsigned char third:3;
  unsigned char fourth:4;
  volatile unsigned vfirst:2;
  volatile unsigned vsecond:3;
  int marker;
  Bits(unsigned a,unsigned b,int n):first(a),second(b),third(a+1),fourth(b+3),vfirst(a),vsecond(b),marker(n){}
};
struct Changed {
  unsigned first:2;
  unsigned second:2;
  Changed();
};
unsigned change(Changed* p) {p->first=2;return 3;}
Changed::Changed():first(1),second(change(this)){}
int main() {
  for(unsigned i=0;i<16;++i){
    Bits a(i,i+1,i),b=a;
    Bits c(static_cast<Bits&&>(b));
    b=a;c=static_cast<Bits&&>(b);
    if(c.first!=(i&1)||c.second!=((i+1)&3)||c.third!=((i+1)&7)||c.fourth!=((i+4)&15))return 1;
    if(c.vfirst!=(i&3)||c.vsecond!=((i+1)&7)||c.marker!=int(i))return 2;
  }
  Changed a;Changed b=a;b=a;
  return b.first!=2||b.second!=3;
}
