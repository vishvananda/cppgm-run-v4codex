auto f() {
  if constexpr(false) { auto lambda=[](){ return 1; }; return lambda(); }
  return 9;
}
int main() { return f()==9 ? 0 : 1; }
