int trace;
struct Active { ~Active() { trace = 1; } };
union Union {
  Active inactive;
  int selected;
  Union() : selected(7) {}
  ~Union() {}
};
struct Empty { Empty() noexcept {} ~Empty() noexcept {} };
int main() {
  { Union u; if (u.selected != 7) return 1; Empty values[2][2]; }
  return trace;
}
