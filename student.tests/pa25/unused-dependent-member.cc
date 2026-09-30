template<class T> struct Item {
  static int value(int x) { return x+3; }
  int unused() { typename T::missing bad; return 0; }
};
int main() { return Item<int>::value(2)-5; }
