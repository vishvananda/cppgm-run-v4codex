int unavailable();
struct Guard { Guard(); ~Guard(); };
int main() {
  if constexpr(false) { static int x=unavailable(); static Guard g; return x; }
  return 0;
}
