// N3485 [class.mi]/4 gives D two distinct B0 subobjects. Neither is virtual.
struct B0 { virtual ~B0() {} };
struct B1 : B0 { ~B1() override {} };
struct B2 : B0 { ~B2() override {} };
struct D : B1, B2 { ~D() override {} };
int main() {
  D d;
  B1* first = &d;
  B2* second = &d;
  B0* a = first;
  B0* b = second;
  return a == b || dynamic_cast<D*>(a) != &d || dynamic_cast<D*>(b) != &d;
}
