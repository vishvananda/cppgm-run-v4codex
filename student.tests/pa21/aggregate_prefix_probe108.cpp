struct S { S(); S(const S&); S(S&&); ~S(); };
static int live, copies, limit;
S::S() { ++live; }
S::S(const S&) { if (++copies==limit) throw copies; ++live; }
S::S(S&&) { if (++copies==limit) throw copies; ++live; }
S::~S() { --live; }
extern "C" int construction_probe();
int main()
{
  for (limit=1; limit<=3; ++limit) {
    copies=live=0;
    try { construction_probe(); return 10; }
    catch (int n) { if (n!=limit) return 11; if (live) return live; }
  }
  limit=0; copies=live=0;
  construction_probe();
  return live;
}
