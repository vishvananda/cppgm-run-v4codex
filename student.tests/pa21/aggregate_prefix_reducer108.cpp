// The PA21 aggregate fixture reduced to its construction ownership boundary.
// Definitions supplied by aggregate_prefix_probe108.cpp exercise each throw point.
struct S { S(); S(const S&); ~S(); };
struct E { S a,b,c; };
extern "C" int construction_probe()
{
  S x,y,z;
  E e={x,y,z};
  (void)e;
  return 0;
}
