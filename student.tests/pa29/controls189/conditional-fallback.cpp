class private_condition { constexpr explicit operator bool() const { return true; } };
struct deleted_condition { explicit operator bool() const = delete; };
struct missing_condition {};
struct runtime_condition { explicit operator bool() const { return true; } };
struct ambiguous_condition {
  constexpr operator int() const { return 1; }
  constexpr operator double() const { return 1; }
};
struct true_condition { constexpr explicit operator bool() const { return true; } };
struct false_condition { constexpr explicit operator bool() const { return false; } };
struct value {
  int selected;
  template<class T> explicit(T{}) value(T) : selected(1) {}
  value(...) : selected(2) {}
};
int main() {
  value a(private_condition{}), b(deleted_condition{}), c(missing_condition{});
  value d(runtime_condition{}), e(ambiguous_condition{}), f(true_condition{});
  value g = true_condition{}, h = false_condition{};
  return a.selected == 2 && b.selected == 2 && c.selected == 2 &&
         d.selected == 2 && e.selected == 2 && f.selected == 1 &&
         g.selected == 2 && h.selected == 1 ? 0 : 1;
}
