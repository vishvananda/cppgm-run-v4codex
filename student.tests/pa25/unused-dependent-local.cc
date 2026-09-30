template<int N> struct Item {
  static int value(int x) { return x+N; }
  int unused() { typename Item<N>::missing bad; return 0; }
};
int main() { return Item<3>::value(2)-5; }
