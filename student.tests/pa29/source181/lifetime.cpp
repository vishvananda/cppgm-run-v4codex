int calls;
struct Element { Element() { ++calls; } Element(const Element&) { ++calls; } Element& operator=(const Element&) { ++calls; return *this; } ~Element() { ++calls; } };
struct S { int tag; Element empty[0]; };
int main() {
  { S a; a.tag = 7; S b = a; b = a; if (b.tag != 7) return 2; }
  return calls;
}
