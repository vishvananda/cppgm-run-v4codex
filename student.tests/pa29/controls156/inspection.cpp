union Bits {double floating;unsigned long long integer;};
double global_nan=__builtin_nans("");
struct Cell {double value;};
Cell global_cell={__builtin_nan("0x11")};
template<class T> int leading(T x) noexcept {return __builtin_clzg(x,93);}
int main() {
  volatile unsigned long long x=0x8000000000000001ULL;
  if(__builtin_popcountll(x)!=2 || leading(x)!=0 || __builtin_ctzll(x)!=0)return 1;
  if(leading((unsigned char)1)!=7)return 2;
  volatile signed char narrow=0;
  if(!__builtin_add_overflow(127,1,&narrow) || narrow!=-128)return 6;
  long long result=0;
  if(!__builtin_mul_overflow((long long)x,2,&result) || result!=2)return 3;
  Bits a={global_nan}; Bits b={-__builtin_nans("")}; Bits c={global_cell.value};
  if(a.integer!=0x7ff4000000000000ULL || b.integer!=0xfff4000000000000ULL || c.integer!=0x7ff8000000000011ULL)return 4;
  volatile double zero=0.0;
  if(!__builtin_signbit(-zero) || __builtin_isgreater(global_nan,zero))return 5;
  return 0;
}
