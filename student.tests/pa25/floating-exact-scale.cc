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
unsigned bits32(float x){unsigned char* p=reinterpret_cast<unsigned char*>(&x);unsigned n=0;for(int i=0;i<4;++i)n|=unsigned(p[i])<<(8*i);return n;}
float value32(unsigned x){float f;unsigned char* p=reinterpret_cast<unsigned char*>(&f);for(int i=0;i<4;++i)p[i]=(unsigned char)(x>>(8*i));return f;}
float scale32(float x,int k){switch(k){case 0:return x*0.5f;case 1:return x*-2.0f;case 2:return x*1.7014118e38f;case 3:return x*1.40129846e-45f;case 4:return x*1.0f;case 5:return x*-1.0f;}return x;}
long double wide32(float x,int k){switch(k){case 0:return (long double)x*(long double)0.5f;case 1:return (long double)x*(long double)-2.0f;case 2:return (long double)x*(long double)1.7014118e38f;case 3:return (long double)x*(long double)1.40129846e-45f;case 4:return (long double)x*(long double)1.0f;case 5:return (long double)x*(long double)-1.0f;}return x;}
int main(int argc,char**){unsigned long long n=argc;
unsigned long long special[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,0x7ff0000000000000ULL,0xfff0000000000000ULL,0x7ff0000000000001ULL,0xfff0000000000001ULL,0x7ff8000000000123ULL,0x7fefffffffffffffULL};
for(int i=0;i<10010;++i){n^=n<<13;n^=n>>7;n^=n<<17;volatile double x=value(i<10?special[i]:n);
for(int k=0;k<6;++k){double a=scale(x,k),b=(double)wide(x,k);if(bits(a)!=bits(b))return 1;}}unsigned special32[]={0,0x80000000U,1,0x80000001U,0x7f800000U,0xff800000U,0x7f800001U,0xff800001U,0x7fc00123U,0x7f7fffffU};
for(int i=0;i<10010;++i){n^=n<<13;n^=n>>7;n^=n<<17;volatile float x=value32(i<10?special32[i]:unsigned(n));
for(int k=0;k<6;++k){float a=scale32(x,k),b=(float)wide32(x,k);if(bits32(a)!=bits32(b))return 2;}}return 0;}
