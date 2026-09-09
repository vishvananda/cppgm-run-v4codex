struct Pick {
  int get() & { return 1; }
  int get() const & { return 2; }
  int get() const && { return 3; }
  int operator()() & { return 4; }
  int operator()() && { return 5; }
};
struct Child : Pick {};
int pointer(const int * const &) { return 6; }
int pointer(const int * &&) { return 7; }
int main() {
  Pick p; const Pick q; Child c; int *n = nullptr;
  if (p.get() != 1 || q.get() != 2 || static_cast<const Pick&&>(q).get() != 3) return 1;
  if (c.get() != 1 || p() != 4 || Pick()() != 5) return 2;
  if (pointer(n) != 7 || pointer(nullptr) != 7) return 3;
  return 0;
}
