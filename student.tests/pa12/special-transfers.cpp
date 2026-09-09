struct Field {
  int value;
  Field(int x = 0) : value(x) {}
  Field(const Field& x) : value(x.value) {}
  Field(Field&& x) : value(x.value) { x.value = -1; }
  Field& operator=(const Field& x) { value = x.value; return *this; }
  Field& operator=(Field&& x) { value = x.value; x.value = -2; return *this; }
};
struct Box { int prefix; Field field; unsigned one:3; unsigned two:5; unsigned:0; unsigned three:4; };
struct Empty {};
struct Derived : Empty { int value; };
int main() {
  Box a; a.prefix=23; a.field.value=19; a.one=5; a.two=17; a.three=11;
  Box b(a);
  if(b.prefix!=23 || b.field.value!=19 || b.one!=5 || b.two!=17 || b.three!=11) return 1;
  Box c(static_cast<Box&&>(a));
  if(c.field.value!=19 || a.field.value!=-1 || c.two!=17) return 2;
  b=c; if(b.field.value!=19 || b.one!=5) return 3;
  b=static_cast<Box&&>(c); if(b.field.value!=19 || c.field.value!=-2) return 4;
  Derived x; Derived y; x.value=7; y.value=9;
  static_cast<Empty&>(x)=static_cast<const Empty&>(y);
  return x.value==7 ? 0 : 5;
}
