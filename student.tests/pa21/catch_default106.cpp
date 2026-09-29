int live;
struct Temporary { Temporary() { ++live; } ~Temporary() { --live; } };
struct Exception {
  Exception() {}
  Exception(const Exception&, const Temporary& = Temporary()) {}
};
int main() {
  try { throw Exception(); }
  catch (Exception value) { return live; }
  return 2;
}
