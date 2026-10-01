int main() {
  int n=0;
  if constexpr (true) { goto target; target: ++n; goto done; }
done:
  return n==1 ? 0 : 1;
}
