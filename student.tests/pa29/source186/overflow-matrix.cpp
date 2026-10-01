// Exact long arithmetic supplies an independent oracle for narrow operands.
template<class T, long Min, long Max> int check() {
 for(long a=-130;a<=130;++a) for(long b=-18;b<=18;++b) {
  T r=0; bool flag=__builtin_add_overflow(a,b,&r);long v=a+b;
  if(flag!=(v<Min||v>Max)||r!=static_cast<T>(v))return 1;
  flag=__builtin_sub_overflow(a,b,&r);v=a-b;
  if(flag!=(v<Min||v>Max)||r!=static_cast<T>(v))return 2;
  flag=__builtin_mul_overflow(a,b,&r);v=a*b;
  if(flag!=(v<Min||v>Max)||r!=static_cast<T>(v))return 3;
 }
 return 0;
}
int main(){return check<unsigned _BitInt(1),0,1>() || check<_BitInt(2),-2,1>() ||
 check<_BitInt(7),-64,63>() || check<unsigned _BitInt(7),0,127>() ||
 check<_BitInt(9),-256,255>() || check<unsigned _BitInt(9),0,511>();}
