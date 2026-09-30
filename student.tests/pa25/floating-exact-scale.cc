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
double scale(double x,int k){switch(k){case 0:return x*0.5;case 1:return x*-2.0;case 2:return x*8.98846567431158e307;case 3:return x*4.9406564584124654e-324;case 4:return x*1.0;case 5:return x*-1.0;}return x;}
long double wide(double x,int k){switch(k){case 0:return (long double)x*(long double)0.5;case 1:return (long double)x*(long double)-2.0;case 2:return (long double)x*(long double)8.98846567431158e307;case 3:return (long double)x*(long double)4.9406564584124654e-324;case 4:return (long double)x*(long double)1.0;case 5:return (long double)x*(long double)-1.0;}return x;}
int main(int argc,char**){unsigned long long n=argc;
unsigned long long special[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff0000000000001ULL,0xfff0000000000001ULL,0x7ff8000000000123ULL,0x7fefffffffffffffULL};
for(int i=0;i<10010;++i){n^=n<<13;n^=n>>7;n^=n<<17;volatile double x=value(i<10?special[i]:n);
for(int k=0;k<6;++k){double a=scale(x,k),b=(double)wide(x,k);if(bits(a)!=bits(b))return 1;}}return 0;}
