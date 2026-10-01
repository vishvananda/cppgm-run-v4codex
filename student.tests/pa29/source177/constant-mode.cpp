int main() {
  if constexpr (int runtime=2; __builtin_is_constant_evaluated()) return runtime==2 ? 0 : 1;
  else return 2;
}
