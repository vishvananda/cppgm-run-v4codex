int trace;
struct Target {
  int value;
  Target(int x) : value(x) { trace = trace * 10 + 1; }
  Target() : Target(9) { trace = trace * 10 + 2; }
};
union Choice {
  int first = 1;
  int second;
  Choice() : second(8) {}
};
struct Holder {
  int before;
  union { int selected; long other; };
  int after;
  int read() { return selected; }
};
int main() {
  Target t; Choice c;
  if (trace != 12 || t.value != 9 || c.second != 8) return 1;
  Holder h = {3, {7}, 5};
  if (h.read() != 7 || h.before != 3 || h.after != 5) return 2;
  union { int value; long unused; };
  value = 6;
  return value == 6 ? 0 : 3;
}
