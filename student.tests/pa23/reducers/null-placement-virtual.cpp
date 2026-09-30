int calls;
struct V {};
struct B : virtual V {
  B() {}
  B(B&) { ++calls; }
  static void* operator new(unsigned long, void*, int) noexcept { return 0; }
};
int main() { B b; B* p = new ((void*)0, 0) B(b); return p != 0 || calls != 0; }
