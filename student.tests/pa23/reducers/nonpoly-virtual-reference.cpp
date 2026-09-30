struct V { int value; };
struct B : virtual V {};
struct C : virtual V { int guard; };
struct D : B, C {};
int read(B& b) { return b.value; }
int main() {
  D d;
  d.guard = 13;
  d.value = 7;
  return read(d) == 7 ? 0 : 1;
}
