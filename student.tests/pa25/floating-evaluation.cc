// Runtime source evaluation must agree with the constexpr excess-precision
// policy. Values are materialized from bytes to keep the operations executable.
unsigned long long bits(double x) {
  unsigned char* p=reinterpret_cast<unsigned char*>(&x);
  unsigned long long n=0; for(int i=0;i<8;++i)n|=(unsigned long long)p[i]<<(8*i);return n;
}
double value(unsigned long long x) {
  double d;unsigned char* p=reinterpret_cast<unsigned char*>(&d);
  for(int i=0;i<8;++i)p[i]=(unsigned char)(x>>(8*i));return d;
}
int main(int argc,char**) {
  unsigned long long a[]={0x000e4a48b30e4a48ULL,0x0009c91ce589c91cULL,0x52d5ffc3f755ffc3ULL,0xc41cd1c7a7ccd1c7ULL,0x373d021dbaad021dULL};
  unsigned long long b[]={0xf46e4bb3ac7e4bb3ULL,0x2ecadde1ca7adde1ULL,0x1730d5868da0d586ULL,0xd4e76c8e89876c8eULL,0xfaed8feca25d8fecULL};
  unsigned long long expected[]={0xb48b0ee4fd9ecb90ULL,0x11274f8051b35b7cULL,0x2a172559d8e6f240ULL,0x5915188be1339834ULL,0xf23acc62e8d1c6e6ULL};
  for(int i=0;i<5;++i){volatile double x=value(a[i]+argc-1),y=value(b[i]);
    double z=i==1?x/y:x*y;if(bits(z)!=expected[i])return i+1;}
  constexpr double x=7.891868843759064e+90;
  constexpr double y=5.631942595176827e-197;
  constexpr double z=x*y;
  volatile double rx=x,ry=y;
  return bits(z)!=bits(rx*ry);
}
